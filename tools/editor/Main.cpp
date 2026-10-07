#include <JuceHeader.h>
#include "MainComponent.h"
#include "DevLogger.h"
#include <juce_gui_basics/juce_gui_basics.h>

class TheKlangEditorApplication : public juce::JUCEApplication {
public:
    TheKlangEditorApplication() {}

    const juce::String getApplicationName() override       { return "The Klang Editor"; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override {
        TKS_LOG_INFO("Initialising The Klang Editor...");
        
        juce::String title = getApplicationName() + " v" + getApplicationVersion();
#if defined(TKF_GIT_COMMIT_COUNT) && defined(TKF_GIT_HASH)
        title << " (Build " << TKF_GIT_COMMIT_COUNT << " - " << TKF_GIT_HASH << ")";
#endif
        mainWindow.reset(new MainWindow(title));
    }

    void shutdown() override {
        TKS_LOG_INFO("Shutting down The Klang Editor...");
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override {
        TKS_LOG_INFO("System requested quit...");
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override {}

    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
           #else
            setResizable(true, true);
            auto area = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
            setBounds(area);
           #endif

            setVisible(true);
            TKS_LOG_INFO("MainWindow visible.");
        }

        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(TheKlangEditorApplication)
