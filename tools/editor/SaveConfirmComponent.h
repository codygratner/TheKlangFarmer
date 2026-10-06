#pragma once
#include <JuceHeader.h>
#include "DiffViewer.h"

class SaveConfirmComponent : public juce::Component {
public:
    SaveConfirmComponent(const juce::String& oldL, const juce::String& newL, const juce::String& oldC, const juce::String& newC,
                         std::function<void(SaveConfirmComponent*)> onCommit, std::function<void(SaveConfirmComponent*)> onCancel)
        : oldLayout(oldL), newLayout(newL), oldControls(oldC), newControls(newC),
          onCommitFn(onCommit), onCancelFn(onCancel)
    {
        commitBtn.setButtonText("Commit Changes");
        cancelBtn.setButtonText("Cancel");
        diffBtn.setButtonText("Show Diff");
        
        addAndMakeVisible(commitBtn);
        addAndMakeVisible(cancelBtn);
        addAndMakeVisible(diffBtn);
        addAndMakeVisible(infoLabel);
        
        infoLabel.setText("You have unsaved JSON changes. Would you like to commit them?", juce::dontSendNotification);
        infoLabel.setJustificationType(juce::Justification::centred);
        
        commitBtn.onClick = [this]() { if (onCommitFn) onCommitFn(this); };
        cancelBtn.onClick = [this]() { if (onCancelFn) onCancelFn(this); };
        diffBtn.onClick = [this]() {
            juce::String oldStr = "=== LAYOUT ===\n" + oldLayout + "\n=== CONTROLS ===\n" + oldControls;
            juce::String newStr = "=== LAYOUT ===\n" + newLayout + "\n=== CONTROLS ===\n" + newControls;
            
            auto* diffViewer = new DiffViewerComponent(oldStr, newStr);
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(diffViewer);
            opts.dialogTitle = "Diff Viewer";
            opts.dialogBackgroundColour = juce::Colour(0xff1e1e1e);
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
        };
        
        setSize(400, 150);
    }
    
    void resized() override {
        auto b = getLocalBounds().reduced(20);
        infoLabel.setBounds(b.removeFromTop(40));
        b.removeFromTop(20);
        
        int btnWidth = b.getWidth() / 3;
        diffBtn.setBounds(b.removeFromLeft(btnWidth).reduced(5));
        commitBtn.setBounds(b.removeFromLeft(btnWidth).reduced(5));
        cancelBtn.setBounds(b.removeFromLeft(btnWidth).reduced(5));
    }
private:
    juce::TextButton commitBtn, cancelBtn, diffBtn;
    juce::Label infoLabel;
    juce::String oldLayout, newLayout, oldControls, newControls;
    std::function<void(SaveConfirmComponent*)> onCommitFn, onCancelFn;
};
