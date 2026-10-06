#include "VersionChecker.h"

VersionChecker& VersionChecker::getInstance()
{
    static VersionChecker instance;
    return instance;
}

VersionChecker::VersionChecker()
    : juce::Thread("TKF_VersionChecker")
{
}

VersionChecker::~VersionChecker()
{
    stopThread(3000);
}

void VersionChecker::checkForUpdates(bool force)
{
    {
        const juce::ScopedLock sl(dataLock);
        if (currentInfo.status == Status::Checking)
            return;

        if (!force && currentInfo.status != Status::Idle)
            return;

        currentInfo.status = Status::Checking;
        currentInfo.errorMessage = {};
    }

    sendChangeMessage();

    if (!isThreadRunning())
        startThread();
    else
        notify();
}

VersionChecker::ReleaseInfo VersionChecker::getReleaseInfo() const
{
    const juce::ScopedLock sl(dataLock);
    return currentInfo;
}

bool VersionChecker::isUpdateAvailable() const
{
    const juce::ScopedLock sl(dataLock);
    return currentInfo.status == Status::UpdateAvailable;
}

juce::File VersionChecker::getPreferencesFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("RlyehSound")
        .getChildFile("TheKlangFarmer")
        .getChildFile("preferences.json");
}

bool VersionChecker::isCheckOnLaunchEnabled()
{
    auto file = getPreferencesFile();
    if (!file.existsAsFile())
        return true;

    auto parsed = juce::JSON::parse(file);
    if (parsed.isObject() && parsed.hasProperty("check_updates_on_launch"))
        return static_cast<bool>(parsed.getProperty("check_updates_on_launch", true));

    return true;
}

void VersionChecker::setCheckOnLaunchEnabled(bool enabled)
{
    auto file = getPreferencesFile();
    file.getParentDirectory().createDirectory();

    juce::var prefsObj(new juce::DynamicObject());
    if (file.existsAsFile()) {
        auto existing = juce::JSON::parse(file);
        if (existing.isObject())
            prefsObj = existing;
    }

    if (auto* dyn = prefsObj.getDynamicObject()) {
        dyn->setProperty("check_updates_on_launch", enabled);
        file.replaceWithText(juce::JSON::toString(prefsObj));
    }
}

int VersionChecker::compareVersionStrings(const juce::String& currentVersion, const juce::String& candidateVersion)
{
    auto parseVersion = [](juce::String str, int& major, int& minor, int& patch, juce::String& suffix) {
        str = str.trim();
        if (str.startsWithIgnoreCase("v"))
            str = str.substring(1);

        int dashIndex = str.indexOfChar('-');
        if (dashIndex >= 0) {
            suffix = str.substring(dashIndex + 1);
            str = str.substring(0, dashIndex);
        } else {
            suffix = {};
        }

        auto tokens = juce::StringArray::fromTokens(str, ".", "");
        major = tokens.size() > 0 ? tokens[0].getIntValue() : 0;
        minor = tokens.size() > 1 ? tokens[1].getIntValue() : 0;
        patch = tokens.size() > 2 ? tokens[2].getIntValue() : 0;
    };

    int curMajor = 0, curMinor = 0, curPatch = 0;
    juce::String curSuffix;
    parseVersion(currentVersion, curMajor, curMinor, curPatch, curSuffix);

    int candMajor = 0, candMinor = 0, candPatch = 0;
    juce::String candSuffix;
    parseVersion(candidateVersion, candMajor, candMinor, candPatch, candSuffix);

    if (curMajor != candMajor)
        return curMajor - candMajor;

    if (curMinor != candMinor)
        return curMinor - candMinor;

    if (curPatch != candPatch)
        return curPatch - candPatch;

    // Same base version numbers: a release without suffix is newer than one with a suffix (e.g. 0.3.0 > 0.3.0-rc1)
    if (curSuffix.isEmpty() && candSuffix.isNotEmpty())
        return 1;
    if (curSuffix.isNotEmpty() && candSuffix.isEmpty())
        return -1;

    return curSuffix.compare(candSuffix);
}

void VersionChecker::setMockRelease(const juce::String& mockTag, const juce::String& mockUrl)
{
    const juce::ScopedLock sl(dataLock);
    mockActive = true;
    mockTagValue = mockTag;
    mockUrlValue = mockUrl;

#ifdef JucePlugin_VersionString
    juce::String currentVer = JucePlugin_VersionString;
#else
    juce::String currentVer = "0.3.0";
#endif

    currentInfo.latestTag = mockTag;
    currentInfo.releaseUrl = mockUrl;
    currentInfo.releaseName = mockTag;
    currentInfo.publishedAt = "2026-10-06T12:00:00Z";
    currentInfo.errorMessage = {};

    if (compareVersionStrings(currentVer, mockTag) < 0)
        currentInfo.status = Status::UpdateAvailable;
    else
        currentInfo.status = Status::UpToDate;

    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        sendSynchronousChangeMessage();
    else
        sendChangeMessage();
}

void VersionChecker::clearMockRelease()
{
    const juce::ScopedLock sl(dataLock);
    mockActive = false;
    mockTagValue = {};
    mockUrlValue = {};
    currentInfo = ReleaseInfo();

    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        sendSynchronousChangeMessage();
    else
        sendChangeMessage();
}

void VersionChecker::run()
{
    while (!threadShouldExit()) {
        bool isMock = false;
        juce::String mTag, mUrl;
        {
            const juce::ScopedLock sl(dataLock);
            isMock = mockActive;
            mTag = mockTagValue;
            mUrl = mockUrlValue;
        }

        if (isMock) {
#ifdef JucePlugin_VersionString
            juce::String currentVer = JucePlugin_VersionString;
#else
            juce::String currentVer = "0.3.0";
#endif
            {
                const juce::ScopedLock sl(dataLock);
                currentInfo.latestTag = mTag;
                currentInfo.releaseUrl = mUrl;
                currentInfo.releaseName = mTag;
                currentInfo.publishedAt = "2026-10-06T12:00:00Z";
                currentInfo.errorMessage = {};
                if (compareVersionStrings(currentVer, mTag) < 0)
                    currentInfo.status = Status::UpdateAvailable;
                else
                    currentInfo.status = Status::UpToDate;
            }
            sendChangeMessage();
            return;
        }

        // Query official GitHub releases API (TheKlangSuite with transition fallback)
        juce::URL url("https://api.github.com/repos/codygratner/TheKlangSuite/releases/latest");
        juce::String extraHeaders = "User-Agent: TheKlangSuite-App\r\nAccept: application/vnd.github.v3+json\r\n";
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withExtraHeaders(extraHeaders)
                           .withConnectionTimeoutMs(4000);

        std::unique_ptr<juce::InputStream> stream(url.createInputStream(options));
        juce::var parsed;
        if (stream != nullptr) {
            auto jsonString = stream->readEntireStreamAsString();
            parsed = juce::JSON::parse(jsonString);
        }

        // Fallback to legacy repo if TheKlangSuite does not exist yet (pre-rename transition)
        if (!parsed.isObject() || (parsed.getDynamicObject() != nullptr && !parsed.getDynamicObject()->hasProperty("tag_name"))) {
            juce::URL fallbackUrl("https://api.github.com/repos/codygratner/TheKlangFarmer/releases/latest");
            std::unique_ptr<juce::InputStream> fallbackStream(fallbackUrl.createInputStream(options));
            if (fallbackStream != nullptr) {
                parsed = juce::JSON::parse(fallbackStream->readEntireStreamAsString());
            }
        }

        if (!parsed.isObject() || (parsed.getDynamicObject() != nullptr && !parsed.getDynamicObject()->hasProperty("tag_name"))) {
            {
                const juce::ScopedLock sl(dataLock);
                currentInfo.status = Status::Error;
                currentInfo.errorMessage = "Unable to connect to GitHub release API.";
            }
            sendChangeMessage();
            return;
        }

        auto* obj = parsed.getDynamicObject();
        juce::String tag = obj->hasProperty("tag_name") ? obj->getProperty("tag_name").toString() : juce::String();
        juce::String releaseUrl = obj->hasProperty("html_url") ? obj->getProperty("html_url").toString() : juce::String();
        juce::String releaseName = obj->hasProperty("name") ? obj->getProperty("name").toString() : tag;
        juce::String publishedAt = obj->hasProperty("published_at") ? obj->getProperty("published_at").toString() : juce::String();

#ifdef JucePlugin_VersionString
        juce::String currentVer = JucePlugin_VersionString;
#else
        juce::String currentVer = "0.3.0";
#endif

        {
            const juce::ScopedLock sl(dataLock);
            currentInfo.latestTag = tag;
            currentInfo.releaseUrl = releaseUrl;
            currentInfo.releaseName = releaseName;
            currentInfo.publishedAt = publishedAt;
            currentInfo.errorMessage = {};

            if (tag.isNotEmpty() && compareVersionStrings(currentVer, tag) < 0)
                currentInfo.status = Status::UpdateAvailable;
            else
                currentInfo.status = Status::UpToDate;
        }

        sendChangeMessage();
        return;
    }
}
