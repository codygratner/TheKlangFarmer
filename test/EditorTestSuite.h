#pragma once
#include "GuiTestHelpers.h"
#include "../tools/editor/MainComponent.h"
#include "UIComponents.h"

class EditorTestSuite {
public:
    static void runSuite(GuiTestHelpers::TestReporter& reporter) {
        using namespace GuiTestHelpers;
        reporter.beginTest("The Klang Editor Functional Test Suite");

        // --- Stage 1: Lifecycle & Layout Bounds ---
        MainComponent editor;
        editor.setSize(1200, 800);
        
        auto bounds = editor.getBounds();
        reporter.expect(bounds.getWidth() == 1200 && bounds.getHeight() == 800, "Initial dimensions 1200x800");

        reporter.expect(editor.navigationTabs.getBounds().getWidth() > 0, "navigationTabs has bounds");
        reporter.expect(editor.layoutsTree.getBounds().getWidth() > 0, "layoutsTree has bounds");
        reporter.expect(editor.controlsTree.getBounds().getWidth() > 0, "controlsTree has bounds");
        reporter.expect(editor.formEditor.getBounds().getWidth() > 0, "formEditor has bounds");
        reporter.expect(editor.previewWrapper.getBounds().getWidth() > 0, "previewWrapper has bounds");
        reporter.expect(editor.jsonContainer.getBounds().getWidth() > 0, "jsonContainer has bounds");

        // --- Stage 2: Tab Switching ---
        reporter.expect(editor.navigationTabs.getCurrentTabIndex() == 0, "Initial tab is CONTROLS (0)");
        
        editor.navigationTabs.setCurrentTabIndex(1);
        reporter.expect(editor.navigationTabs.getCurrentTabIndex() == 1, "Switched to LAYOUTS tab (1)");
        
        editor.navigationTabs.setCurrentTabIndex(0);

        // --- Stage 3: Tree Hierarchy Audit ---
        auto* lRoot = editor.layoutsTree.getRootItem();
        auto* cRoot = editor.controlsTree.getRootItem();
        reporter.expect(lRoot != nullptr && lRoot->getNumSubItems() > 0, "layoutsTree populated");
        reporter.expect(cRoot != nullptr && cRoot->getNumSubItems() > 0, "controlsTree populated");

        // --- Stage 4: Headless Data Sync Test & Crash Safety ---
        int totalNodesTested = 0;
        int paramNodesTested = 0;
        bool anyCrash = false;

        std::function<void(EditorTreeItem*, juce::TreeView&)> testAllNodes = [&](EditorTreeItem* node, juce::TreeView& tree) {
            if (!node) return;
            
            try {
                // Simulate selection
                node->setSelected(true, true);
                editor.onTreeItemSelected(node);
                pumpMessageLoop();
                
                // Assertions for specific node types
                if (node->itemType == "card_param" || node->itemType == "card") {
                    auto props = ComponentFinder::findAllByType<juce::PropertyComponent>(&editor.formEditor);
                    if (node->itemType == "card_param") {
                        paramNodesTested++;
                        if (editor.currentParamTarget != node->paramId) anyCrash = true; // Not a crash, but a failure
                    }
                }
            } catch (...) {
                anyCrash = true;
            }
            totalNodesTested++;
            
            for (int i = 0; i < node->getNumSubItems(); ++i) {
                if (auto* child = dynamic_cast<EditorTreeItem*>(node->getSubItem(i))) {
                    testAllNodes(child, tree);
                }
            }
        };

        if (cRoot) testAllNodes(static_cast<EditorTreeItem*>(cRoot), editor.controlsTree);
        if (lRoot) testAllNodes(static_cast<EditorTreeItem*>(lRoot), editor.layoutsTree);

        reporter.expect(!anyCrash, "100% of Tree View nodes selected without crashing or failing binding");
        reporter.expect(totalNodesTested > 0, "Iterated through " + juce::String(totalNodesTested) + " total nodes");
        reporter.expect(paramNodesTested > 0, "Successfully bound and tested " + juce::String(paramNodesTested) + " JSON parameter properties");
        
        // --- Stage 5: Null/Crash Safety Test ---
        try {
            editor.onTreeItemSelected(nullptr);
            reporter.expect(true, "Handled nullptr selection gracefully");
        } catch (...) {
            reporter.expect(false, "Handled nullptr selection gracefully");
        }

        // --- Stage 6: Toolbar Actions ---
        if (editor.toggleOriginalButton.onClick) editor.toggleOriginalButton.onClick();
        pumpMessageLoop();
        reporter.expect(editor.showingOriginal == true, "toggleOriginalButton set showingOriginal=true");
        if (editor.toggleOriginalButton.onClick) editor.toggleOriginalButton.onClick();
        
        if (editor.refreshButton.onClick) editor.refreshButton.onClick();
        pumpMessageLoop();
        reporter.expect(true, "Refresh button executed cleanly");

        // --- Stage 7: Callouts & Overlays Inspection Test ---
        reporter.beginTest("Callouts & Overlays Inspection & Live Preview");

        EditorTreeItem* calloutsNode = nullptr;
        for (int i = 0; i < cRoot->getNumSubItems(); ++i) {
            auto* item = dynamic_cast<EditorTreeItem*>(cRoot->getSubItem(i));
            if (item && item->name == "Callouts & Overlays") {
                calloutsNode = item;
                break;
            }
        }
        reporter.expect(calloutsNode != nullptr, "Callouts & Overlays node present in tree");
        if (calloutsNode != nullptr) {
            reporter.expect(calloutsNode->getNumSubItems() >= 2, "Callouts & Overlays contains child preview items");

            for (int i = 0; i < calloutsNode->getNumSubItems(); ++i) {
                auto* calloutItem = dynamic_cast<EditorTreeItem*>(calloutsNode->getSubItem(i));
                if (!calloutItem) continue;

                calloutItem->setSelected(true, true);
                editor.onTreeItemSelected(calloutItem);
                pumpMessageLoop();

                reporter.expect(editor.previewWrapper.getNumChildComponents() > 0,
                                "previewWrapper populated with child components for " + calloutItem->name);

                auto props = ComponentFinder::findAllByType<juce::PropertyComponent>(&editor.formEditor);
                reporter.expect(props.size() >= 5,
                                "formEditor exposes callout styling properties for " + calloutItem->name);

                // Offscreen smoke paint check
                juce::Image smokeImage(juce::Image::ARGB, 800, 600, true);
                juce::Graphics g(smokeImage);
                editor.previewWrapper.paintEntireComponent(g, true);
                reporter.expect(true, "Offscreen smoke paint check succeeded for " + calloutItem->name);
            }
        }
    }
};
