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

        // --- Stage 8: Verify Master Limiter exists under The Klang Planter in the tree ---
        reporter.beginTest("Editor Stage 8: Master Limiter in The Klang Planter Tree");
        EditorTreeItem* tkpNode = nullptr;
        for (int i = 0; i < cRoot->getNumSubItems(); ++i) {
            auto* item = dynamic_cast<EditorTreeItem*>(cRoot->getSubItem(i));
            if (item && item->productId == "tkp") {
                tkpNode = item;
                break;
            }
        }
        reporter.expect(tkpNode != nullptr, "The Klang Planter node present in controls tree");
        EditorTreeItem* limiterUnderTkp = nullptr;
        if (tkpNode != nullptr) {
            for (int i = 0; i < tkpNode->getNumSubItems(); ++i) {
                auto* item = dynamic_cast<EditorTreeItem*>(tkpNode->getSubItem(i));
                if (item && item->name.contains("Master Limiter")) {
                    limiterUnderTkp = item;
                    break;
                }
            }
            reporter.expect(limiterUnderTkp != nullptr, "Master Limiter node found under The Klang Planter");
            if (limiterUnderTkp != nullptr) {
                reporter.expect(limiterUnderTkp->getNumSubItems() == 4, "Master Limiter under TKP has 4 child parameters");
            }
        }

        // --- Stage 9: Verify Planter Master Limiter callout has 4 child parameter tree items ---
        reporter.beginTest("Editor Stage 9: Planter Master Limiter Callout Child Parameters");
        EditorTreeItem* calloutsRoot = nullptr;
        for (int i = 0; i < cRoot->getNumSubItems(); ++i) {
            auto* item = dynamic_cast<EditorTreeItem*>(cRoot->getSubItem(i));
            if (item && item->name == "Callouts & Overlays") {
                calloutsRoot = item;
                break;
            }
        }
        reporter.expect(calloutsRoot != nullptr, "Callouts & Overlays root node present");
        if (calloutsRoot != nullptr) {
            EditorTreeItem* planterLimiterCallout = nullptr;
            for (int i = 0; i < calloutsRoot->getNumSubItems(); ++i) {
                auto* item = dynamic_cast<EditorTreeItem*>(calloutsRoot->getSubItem(i));
                if (item && item->name.contains("Master Limiter")) {
                    planterLimiterCallout = item;
                    break;
                }
            }
            reporter.expect(planterLimiterCallout != nullptr, "Planter Master Limiter callout node present");
            if (planterLimiterCallout != nullptr) {
                reporter.expect(planterLimiterCallout->getNumSubItems() == 4,
                                "Planter Master Limiter callout has exactly 4 child parameters");
            }
        }

        // --- Stage 10: Where Used Panel & Double-Click Navigation ---
        reporter.beginTest("Editor Stage 10: Where-Used Panel & Double-Click Navigation");
        reporter.expect(editor.whereUsedListBox.getBounds().getWidth() > 0, "whereUsedListBox has bounds");
        reporter.expect(editor.whereUsedLabel.getBounds().getWidth() > 0, "whereUsedLabel has bounds");

        // Test selection of a parameter node: e.g. first child of limiterUnderTkp
        if (limiterUnderTkp != nullptr && limiterUnderTkp->getNumSubItems() > 0) {
            auto* paramItem = dynamic_cast<EditorTreeItem*>(limiterUnderTkp->getSubItem(0));
            reporter.expect(paramItem != nullptr, "Got parameter child of Master Limiter");
            if (paramItem != nullptr) {
                paramItem->setSelected(true, true);
                editor.onTreeItemSelected(paramItem);
                pumpMessageLoop();

                reporter.expect(editor.whereUsedModel.getNumRows() > 0,
                                "whereUsedListBox populated with references for " + paramItem->paramId);

                // Double click first reference in list to test navigation
                editor.whereUsedModel.triggerDoubleClick(0);
                pumpMessageLoop();
                reporter.expect(true, "Double-click navigation executed without crashing");
            }
        }

        // Test selection of callout node
        if (calloutsRoot != nullptr && calloutsRoot->getNumSubItems() > 0) {
            auto* calloutItem = dynamic_cast<EditorTreeItem*>(calloutsRoot->getSubItem(0));
            if (calloutItem != nullptr) {
                calloutItem->setSelected(true, true);
                editor.onTreeItemSelected(calloutItem);
                pumpMessageLoop();

                reporter.expect(editor.whereUsedModel.getNumRows() > 0,
                                "whereUsedListBox populated with parameters for " + calloutItem->name);

                // Double click first parameter reference to test navigation
                editor.whereUsedModel.triggerDoubleClick(0);
                pumpMessageLoop();
                reporter.expect(true, "Double-click navigation from callout executed without crashing");
            }
        }

        // --- Stage 11: Top-Level Callout Parameter Controls in Form Editor ---
        reporter.beginTest("Editor Stage 11: Top-Level Callout Parameter Controls in Form Editor");
        if (limiterUnderTkp != nullptr) {
            limiterUnderTkp->setSelected(true, true);
            editor.onTreeItemSelected(limiterUnderTkp);
            pumpMessageLoop();

            auto props = ComponentFinder::findAllByType<juce::PropertyComponent>(&editor.formEditor);
            reporter.expect(props.size() > 10, "formEditor populated with style and parameter controls for top-level callout");

            bool hasGain = false, hasThresh = false, hasRelease = false, hasEnable = false;
            for (auto* prop : props) {
                auto name = prop->getName();
                if (name.containsIgnoreCase("Gain")) hasGain = true;
                if (name.containsIgnoreCase("Threshold") || name.containsIgnoreCase("Thresh")) hasThresh = true;
                if (name.containsIgnoreCase("Release")) hasRelease = true;
                if (name.containsIgnoreCase("Enable")) hasEnable = true;
            }
            reporter.expect(hasGain, "formEditor exposes Limiter Gain control");
            reporter.expect(hasThresh, "formEditor exposes Limiter Threshold control");
            reporter.expect(hasRelease, "formEditor exposes Limiter Release control");
            reporter.expect(hasEnable, "formEditor exposes Limit Enable control");

            // Verify controlsJsonDocument and paramToFileMap
            auto docContent = editor.controlsJsonDocument.getAllContent();
            reporter.expect(docContent.contains("planter_limiter_gain"), "controlsJsonDocument contains planter_limiter_gain");
            reporter.expect(editor.paramToFileMap["planter_limiter_gain"] == "planter.json", "paramToFileMap maps planter_limiter_gain to planter.json");
        }
    }
};
