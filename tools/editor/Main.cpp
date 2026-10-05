#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"
#include <JuceHeader.h> // for ProjectInfo::versionString

class TheKlangEditorApplication : public juce::JUCEApplication {
public:
    TheKlangEditorApplication() {}

    const juce::String getApplicationName() override       { return "The Klang Editor"; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override {
        mainWindow.reset(new MainWindow(getApplicationName() + " v" + getApplicationVersion()));
    }

    void shutdown() override {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override {
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
            setFullScreen(true); // Maximized on start
            centreWithSize(getWidth(), getHeight());
           #endif

            setVisible(true);
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

