#define JUCE_LOG_ASSERTIONS 1
#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h> // for ProjectInfo::versionString

class TheKlangEditorApplication : public juce::JUCEApplication {
public:
    TheKlangEditorApplication() {}

    const juce::String getApplicationName() override       { return "The Klang Editor"; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override {
        juce::FileLogger::createDefaultAppLogger("TheKlangFarmer", "Editor.log", "App Started");
        juce::Logger::writeToLog("Initialising...");
        mainWindow.reset(new MainWindow(getApplicationName() + " v" + getApplicationVersion()));
    }

    void shutdown() override {
        juce::Logger::writeToLog("Shutting down...");
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override {
        juce::Logger::writeToLog("System requested quit...");
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
            juce::Logger::writeToLog("MainWindow visible.");
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
