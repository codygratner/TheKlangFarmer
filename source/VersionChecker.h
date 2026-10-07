#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class VersionChecker : public juce::Thread,
                       public juce::ChangeBroadcaster,
                       public juce::DeletedAtShutdown
{
public:
    enum class Status
    {
        Idle,
        Checking,
        UpToDate,
        UpdateAvailable,
        Error
    };

    struct ReleaseInfo
    {
        Status status = Status::Idle;
        juce::String latestTag;
        juce::String releaseUrl;
        juce::String releaseName;
        juce::String publishedAt;
        juce::String errorMessage;
    };

    static VersionChecker& getInstance();
    static void teardown();

    ~VersionChecker() override;

    void checkForUpdates(bool force = false);
    ReleaseInfo getReleaseInfo() const;
    bool isUpdateAvailable() const;

    static bool isCheckOnLaunchEnabled();
    static void setCheckOnLaunchEnabled(bool enabled);

    static int compareVersionStrings(const juce::String& currentVersion, const juce::String& candidateVersion);

    void setMockRelease(const juce::String& mockTag, const juce::String& mockUrl);
    void clearMockRelease();

private:
    VersionChecker();

    void run() override;
    static juce::File getPreferencesFile();

    mutable juce::CriticalSection dataLock;
    ReleaseInfo currentInfo;
    std::atomic<bool> checkRequested { false };
    bool mockActive = false;
    juce::String mockTagValue;
    juce::String mockUrlValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VersionChecker)
};
