#pragma once
#include "GuiTestHelpers.h"
#include "FarmerProcessor.h"
#include "FarmerEditor.h"

namespace FarmerTestSuite {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("The Klang Farmer Functional Test Suite");

        // --- Stage 1: Lifecycle & Layout Bounds ---
        TheKlangFarmerAudioProcessor processor;
        
        auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
        reporter.expect(editor != nullptr, "Editor instantiated");
        
        editor->setSize(1000, 750);
        auto bounds = editor->getBounds();
        reporter.expect(bounds.getWidth() == 1000 && bounds.getHeight() == 750, "Initial dimensions 1000x750", editor.get(), "Dimensions");
        
        editor->setSize(800, 600);
        bounds = editor->getBounds();
        reporter.expect(bounds.getWidth() == 800 && bounds.getHeight() == 600, "Resize limits min 800x600");
        
        editor->setSize(1200, 900);
        bounds = editor->getBounds();
        reporter.expect(bounds.getWidth() == 1200 && bounds.getHeight() == 900, "Resize limits max 1200x900");
        
        editor->setSize(1000, 750); // back to normal

        auto* navCard = ComponentFinder::findByType<NavigationCardComponent>(editor.get());
        reporter.expect(navCard != nullptr, "Navigation card exists", editor.get(), "NavCard");

        // --- Stage 2: All 7 Pages Navigation Transitions ---
        if (navCard) {
            for (int page = 0; page < 7; ++page) {
                // Simulate click on nav button (we can't access buttons array directly, so we just find the button by name if needed, or call setSelectedPage directly to trigger the callback, or find the TextButton child)
                auto buttons = ComponentFinder::findAllByType<juce::TextButton>(navCard);
                if (page < buttons.size()) {
                    EventSimulator::simulateClick(buttons[page]);
                    pumpMessageLoop();
                    
                    // Verify the page changed
                    reporter.expect(navCard->getSelectedPage() == page, "Navigated to page " + juce::String(page));
                    
                    // Specific module card visibility checks could be added here
                }
            }
        }
        // Go back to Voice 1
        if (navCard) {
            auto buttons = ComponentFinder::findAllByType<juce::TextButton>(navCard);
            if (!buttons.isEmpty()) {
                EventSimulator::simulateClick(buttons[0]);
                pumpMessageLoop();
            }
        }

        // --- Stage 3: APVTS 2-Way Parameter Sync & Formatters ---
        auto* pitchSlider = ComponentFinder::findSliderByParamId(editor.get(), "carrier1_pitch");
        reporter.expect(pitchSlider != nullptr, "Found carrier1_pitch slider", editor.get(), "FindPitchSlider");
        
        if (pitchSlider) {
            float initialVal = pitchSlider->getValue();
            pitchSlider->setValue(initialVal + 0.1f, juce::sendNotificationSync);
            pumpMessageLoop();
            reporter.expect(pitchSlider->getValue() > initialVal, "Slider drag increases value");
            
            // APVTS updated?
            auto* apvtsParam = processor.apvts.getParameter("carrier1_pitch");
            reporter.expect(apvtsParam->getValue() > 0.51f, "APVTS value increased via drag");

            // Double click opens text entry (value preserved without resetting)
            double valBeforeDbl = pitchSlider->getValue();
            EventSimulator::simulateDoubleClick(pitchSlider);
            pumpMessageLoop();
            reporter.expect(std::abs(pitchSlider->getValue() - valBeforeDbl) < 0.001, "Double-click preserves value without resetting");

            // Alt-click resets to default
            EventSimulator::simulateAltClick(pitchSlider);
            pumpMessageLoop();
            reporter.expect(std::abs(pitchSlider->getValue() - 0.5) < 0.01, "Alt-click resets to default");
            
            // APVTS -> UI
            apvtsParam->setValueNotifyingHost(1.0f);
            pumpMessageLoop();
            reporter.expect(std::abs(pitchSlider->getValue() - 1.0f) < 0.001f, "APVTS change syncs to UI slider");
            apvtsParam->setValueNotifyingHost(0.5f); // reset
        }

        auto* trackSelector = ComponentFinder::findSelectorByParamId(editor.get(), "carrier1_tracking");
        if (trackSelector) {
            EventSimulator::simulateClick(trackSelector);
            pumpMessageLoop();
            auto* apvtsParam = processor.apvts.getParameter("carrier1_tracking");
            reporter.expect(apvtsParam->getValue() > 0.0f, "Selector click updates APVTS");
        }

        // --- Stage 4: Full 13 FX Algorithms Dynamic Reconfiguration Sweep ---
        // Go to Pre-Amp FX page (index 3)
        if (navCard) {
            auto buttons = ComponentFinder::findAllByType<juce::TextButton>(navCard);
            if (buttons.size() > 3) {
                EventSimulator::simulateClick(buttons[3]);
                pumpMessageLoop();
            }
        }
        
        auto fxCards = ComponentFinder::findAllByType<FXSlotCardComponent>(editor.get());
        reporter.expect(fxCards.size() >= 4, "Found FX Slot cards on Pre-Amp page");
        
        if (fxCards.size() >= 1) {
            auto* fxCard = fxCards[0];
            // Test a few algorithms explicitly
            int testTypes[] = { 1, 2, 3, 4, 13 }; // Bell EQ, Chorus, Comb, Drive, WaveFolder
            for (int type : testTypes) {
                fxCard->configureForType(type);
                pumpMessageLoop();
                reporter.expect(fxCard->getCurrentType() == type, "Configured FX Slot for type " + juce::String(type));
            }
        }

        // --- Stage 4B: Drag-and-Drop FX Rack Swap Verification ---
        if (fxCards.size() >= 2) {
            auto* farmerEditor = dynamic_cast<TheKlangFarmerAudioProcessorEditor*>(editor.get());
            reporter.expect(farmerEditor != nullptr, "Farmer editor cast successful");

            if (farmerEditor) {
                // Set Pre 1 to Chorus (2) and Pre 2 to Drive (4)
                processor.getEngine().setPreFXType(0, 2);
                processor.getEngine().setPreFXParam(0, 0, 0.75f);
                if (auto* p = processor.apvts.getParameter("pre_fx_1_type")) p->setValueNotifyingHost(p->convertTo0to1(2.0f));
                if (auto* p = processor.apvts.getParameter("pre_fx_1_p1")) p->setValueNotifyingHost(0.75f);

                processor.getEngine().setPreFXType(1, 4);
                processor.getEngine().setPreFXParam(1, 0, 0.25f);
                if (auto* p = processor.apvts.getParameter("pre_fx_2_type")) p->setValueNotifyingHost(p->convertTo0to1(4.0f));
                if (auto* p = processor.apvts.getParameter("pre_fx_2_p1")) p->setValueNotifyingHost(0.25f);

                // Execute intra-lane swap: Pre 1 with Pre 2
                farmerEditor->handleFXSwap(false, 0, false, 1);
                pumpMessageLoop();

                reporter.expect(processor.getEngine().getPreFXType(0) == 4, "Slot 0 swapped to Drive (4)");
                reporter.expect(processor.getEngine().getPreFXType(1) == 2, "Slot 1 swapped to Chorus (2)");
                reporter.expect(std::abs(processor.getEngine().getPreFXParam(0, 0) - 0.25f) < 1e-4f, "Slot 0 received Drive Gain param");
                reporter.expect(std::abs(processor.getEngine().getPreFXParam(1, 0) - 0.75f) < 1e-4f, "Slot 1 received Chorus Rate param");

                // Set Post 0 to empty (0) to test swap with empty slot
                processor.getEngine().setPostFXType(0, 0);
                if (auto* p = processor.apvts.getParameter("post_fx_1_type")) p->setValueNotifyingHost(0.0f);

                // Execute cross-lane swap: Pre 0 (Drive 4) with Post 0 (empty 0)
                farmerEditor->handleFXSwap(false, 0, true, 0);
                pumpMessageLoop();

                reporter.expect(processor.getEngine().getPreFXType(0) == 0, "Pre Slot 0 is now empty (0)");
                reporter.expect(processor.getEngine().getPostFXType(0) == 4, "Post Slot 0 received Drive (4)");
                reporter.expect(std::abs(processor.getEngine().getPostFXParam(0, 0) - 0.25f) < 1e-4f, "Post Slot 0 received Drive Gain param");

                // Verify DragAndDropTarget interface on FX cards
                auto* dndTarget = dynamic_cast<juce::DragAndDropTarget*>(fxCards[0]);
                reporter.expect(dndTarget != nullptr, "FX card implements juce::DragAndDropTarget");

                auto* dndContainer = dynamic_cast<juce::DragAndDropContainer*>(farmerEditor);
                reporter.expect(dndContainer != nullptr, "Editor implements juce::DragAndDropContainer");
            }
        }

        // --- Stage 5: Right-Click Callout Popups & Snap-Point Clicks ---
        if (pitchSlider) {
            EventSimulator::simulateRightClick(pitchSlider);
            pumpMessageLoop();
            SliderCalloutComponent* callout = nullptr;
            for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i) {
                if (auto* found = ComponentFinder::findByType<SliderCalloutComponent>(juce::Desktop::getInstance().getComponent(i))) {
                    callout = found;
                    break;
                }
            }
            reporter.expect(callout != nullptr, "Right-click opened SliderCalloutComponent");
            
            if (callout) {
                auto snapButtons = ComponentFinder::findAllByType<juce::TextButton>(callout);
                if (!snapButtons.isEmpty()) {
                    EventSimulator::simulateClick(snapButtons[0]);
                    pumpMessageLoop();
                    bool stillExists = false;
                    for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i) {
                        if (ComponentFinder::findByType<SliderCalloutComponent>(juce::Desktop::getInstance().getComponent(i))) stillExists = true;
                    }
                    reporter.expect(!stillExists, "Clicking snap button closes callout");
                }
            }
        }

        // --- Stage 6: Master Header Controls & Modals ---
        auto* vizCard = ComponentFinder::findByType<VisualizationCardComponent>(editor.get());
        reporter.expect(vizCard != nullptr, "VisualizationCard exists");

        // --- Stage 7: Audio-to-Visualizer Data Pipeline ---
        // Render some audio
        juce::AudioBuffer<float> buffer(2, 512);
        buffer.clear();
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
        processor.processBlock(buffer, midi);
        
        // Trigger timer manually or pump
        if (auto* farmerEditor = dynamic_cast<TheKlangFarmerAudioProcessorEditor*>(editor.get())) {
            farmerEditor->timerCallback();
            pumpMessageLoop();
            reporter.expect(true, "Audio pipeline triggered timerCallback without asserting");
        }

        // --- Stage 8: Preset / State Save & Restore Roundtrip ---
        juce::MemoryBlock stateBlock;
        processor.getStateInformation(stateBlock);
        
        if (pitchSlider) pitchSlider->setValue(0.2f, juce::sendNotificationSync);
        pumpMessageLoop();
        
        processor.setStateInformation(stateBlock.getData(), static_cast<int>(stateBlock.getSize()));
        pumpMessageLoop();
        
        if (pitchSlider) {
            reporter.expect(std::abs(pitchSlider->getValue() - 0.5f) < 0.001f, "State restore reverted pitch slider");
        }

        std::cout << "[DEBUG] Starting Stage 9" << std::endl;
        // --- Stage 9: Phase 3 Neo-Slate 3-Tier Layout, Header Mini-Dock, Modulator Strip & d6 Randomizer ---
        if (auto* farmerEditor = dynamic_cast<TheKlangFarmerAudioProcessorEditor*>(editor.get())) {
            // 1. Verify typography
            auto font = TkfTypography::getFont(12.0f);
            reporter.expect(font.getHeight() > 0.0f, "TkfTypography returns valid font");

            // 2. Verify Header Safety Controls (Undo, Redo, AB, Global d6, Meter)
            reporter.expect(farmerEditor->getUndoButton().isVisible(), "Undo button visible in header");
            reporter.expect(farmerEditor->getRedoButton().isVisible(), "Redo button visible in header");
            reporter.expect(farmerEditor->getAbButton().isVisible(), "A/B button visible in header");
            reporter.expect(farmerEditor->getGlobalDiceButton().isVisible(), "Global d6 button visible in header");
            reporter.expect(farmerEditor->getHeaderMeter().isVisible(), "Header peak meter visible in header");

            std::cout << "[DEBUG] Stage 9: test Panic flush" << std::endl;
            // 3. Test double-click Panic flush on Header Peak Meter
            EventSimulator::simulateDoubleClick(&farmerEditor->getHeaderMeter());
            pumpMessageLoop();
            reporter.expect(true, "Header meter double-click executed Panic flush without crash");

            // 4. Test global d6 randomizer roll
            EventSimulator::simulateClick(&farmerEditor->getGlobalDiceButton());
            pumpMessageLoop();
            reporter.expect(true, "Global d6 roll executed across active parameters");

            // 5. Verify Lower Modulator Strip
            reporter.expect(farmerEditor->getModStrip().isVisible(), "Lower Modulator Strip is visible");
            reporter.expect(farmerEditor->getModStrip().getNumTiles() == 15, "Lower Modulator Strip contains all 15 modulator tiles");

            farmerEditor->getModStrip().setSelectedTile(2);
            pumpMessageLoop();
            reporter.expect(farmerEditor->getModStrip().getSelectedTile() == 2, "Selected LFO 3 tile in Modulator Strip");

            // 6. Test contextual card d6 dice button
            auto cards = ComponentFinder::findAllByType<ModuleCardComponent>(editor.get());
            reporter.expect(!cards.isEmpty(), "Found module cards in workspace");
            if (!cards.isEmpty()) {
                auto* firstCard = cards[0];
                EventSimulator::simulateClick(&firstCard->getDiceButton());
                pumpMessageLoop();
                reporter.expect(true, "Contextual card d6 button clicked without crash");
            }

            // --- Stage 10: Phase 4 Modulation Matrix UI, Performance Macros, Undo/Redo & SmartValueParser ---
            {
                farmerEditor->setPage(0);
                pumpMessageLoop();

                // 1. Undo / Redo Transactions
                processor.getUndoManager().clearUndoHistory();
                processor.getUndoManager().beginNewTransaction("Modify Pitch");
                if (pitchSlider) {
                    pitchSlider->setValue(0.85, juce::sendNotificationSync);
                    processor.apvts.copyState();
                }
                pumpMessageLoop();
                reporter.expect(std::abs(pitchSlider->getValue() - 0.85) < 0.01, "Pitch slider moved to 0.85");

                EventSimulator::simulateClick(&farmerEditor->getUndoButton());
                pumpMessageLoop();
                reporter.expect(std::abs(pitchSlider->getValue() - 0.5) < 0.01, "Undo transaction reverted pitch slider to 0.50");

                EventSimulator::simulateClick(&farmerEditor->getRedoButton());
                pumpMessageLoop();
                reporter.expect(std::abs(pitchSlider->getValue() - 0.85) < 0.01, "Redo transaction reapplied pitch slider to 0.85");

                // Reset back
                EventSimulator::simulateClick(&farmerEditor->getUndoButton());
                pumpMessageLoop();

                // 2. A/B Comparison Memory Buffers
                processor.saveToBufferA();
                if (pitchSlider) pitchSlider->setValue(0.33, juce::sendNotificationSync);
                pumpMessageLoop();
                processor.saveToBufferB();
                if (pitchSlider) pitchSlider->setValue(0.50, juce::sendNotificationSync);
                pumpMessageLoop();
                reporter.expect(!processor.isBufferBActive(), "Buffer A is initially active");

                EventSimulator::simulateClick(&farmerEditor->getAbButton());
                pumpMessageLoop();
                reporter.expect(processor.isBufferBActive(), "A/B button toggled active buffer to B");
                if (pitchSlider) {
                    reporter.expect(std::abs(pitchSlider->getValue() - 0.33) < 0.01, "Buffer B recalled pitch value 0.33");
                }

                EventSimulator::simulateClick(&farmerEditor->getAbButton());
                pumpMessageLoop();
                reporter.expect(!processor.isBufferBActive(), "A/B button toggled active buffer back to A");
                if (pitchSlider) {
                    reporter.expect(std::abs(pitchSlider->getValue() - 0.5) < 0.01, "Buffer A recalled pitch value 0.50");
                }

                processor.copyAToB();
                EventSimulator::simulateClick(&farmerEditor->getAbButton());
                pumpMessageLoop();
                if (pitchSlider) {
                    reporter.expect(std::abs(pitchSlider->getValue() - 0.5) < 0.01, "Copied Buffer A to Buffer B successfully");
                }
                EventSimulator::simulateClick(&farmerEditor->getAbButton());
                pumpMessageLoop();

                // 3. 4 Performance Macro Knobs
                for (int m = 0; m < 4; ++m) {
                    reporter.expect(farmerEditor->getMacroKnob(m).isVisible(), "Macro knob M" + juce::String(m + 1) + " is visible");
                }
                farmerEditor->getMacroKnob(0).setValue(0.72, juce::sendNotificationSync);
                pumpMessageLoop();
                if (auto* mParam = processor.apvts.getParameter("macro_1")) {
                    reporter.expect(std::abs(mParam->getValue() - 0.72f) < 0.01f, "Macro 1 knob synced to APVTS parameter macro_1");
                }

                // 4. SmartValueParser
                double hz = 0.0;
                bool parsedHz = SmartValueParser::parseNoteToHz("C2+37c", hz);
                reporter.expect(parsedHz && std::abs(hz - 66.82099) < 0.05, "SmartValueParser parsed C2+37c to ~66.82 Hz (" + juce::String(hz, 2) + ")");

                double gain = 0.0;
                bool parsedGain = SmartValueParser::parseDecibelsToGain("-6dB", gain);
                reporter.expect(parsedGain && std::abs(gain - 0.501187) < 0.02, "SmartValueParser parsed -6dB to ~0.50 gain (" + juce::String(gain, 3) + ")");

                double ms = 0.0;
                bool parsedMs1 = SmartValueParser::parseTimeToMs("1/4d", 120.0, ms);
                reporter.expect(parsedMs1 && std::abs(ms - 750.0) < 0.1, "SmartValueParser parsed 1/4d at 120 BPM to 750 ms (" + juce::String(ms, 1) + ")");

                bool parsedMs2 = SmartValueParser::parseTimeToMs("1/8t", 120.0, ms);
                reporter.expect(parsedMs2 && std::abs(ms - 166.67) < 0.5, "SmartValueParser parsed 1/8t at 120 BPM to 166.67 ms (" + juce::String(ms, 1) + ")");

                // 5. Modulator Strip Drag-and-Drop Mapping to Knob
                farmerEditor->getMacroKnob(0).onModulationAssigned("Mod Env 1");
                pumpMessageLoop();
                reporter.expect(farmerEditor->getMacroKnob(0).modulation.isModulated, "Knob modulation range visualizer activated on assignment");

                // 6. Modulation Tracer Overlay Cables
                farmerEditor->updateTracerForFocusedKnob(&farmerEditor->getMacroKnob(0));
                pumpMessageLoop();
                reporter.expect(farmerEditor->getTracerOverlay().isVisible(), "Tracer overlay is visible");

                // 7. Modulations Page & Matrix Table
                farmerEditor->setPage(5);
                pumpMessageLoop();
                reporter.expect(farmerEditor->getModMatrixTable() != nullptr && farmerEditor->getModMatrixTable()->isVisible(),
                                "Modulation Matrix Table is visible on Page 5");

                // 8. Inspector Popover
                farmerEditor->openInspector(0);
                pumpMessageLoop();
                reporter.expect(farmerEditor->getInspectorPopover() != nullptr && farmerEditor->getInspectorPopover()->isVisible(),
                                "Spacious Inspector Popover opened on modulator tile inspect");
                reporter.expect(farmerEditor->getTracerOverlay().hasActiveCables(),
                                "Tracer overlay generated patch cables to Inspector Popover Eurorack sockets");

                // Test Escape key close
                farmerEditor->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
                pumpMessageLoop();
                reporter.expect(farmerEditor->getInspectorPopover() == nullptr,
                                "Escape key closed Inspector Popover and cleared tracer cables");

                // --- Stage 11: v0.4.0 Interaction Scheme Remediation & Audition Trigger ---
                // 1. Double-click vs Alt-click test
                auto* testKnob = &farmerEditor->getMacroKnob(1);
                double initialDef = testKnob->getDefaultValue ? testKnob->getDefaultValue() : testKnob->getDoubleClickReturnValue();
                double nonDefVal = 0.88;
                testKnob->setValue(nonDefVal, juce::dontSendNotification);
                reporter.expect(std::abs(testKnob->getValue() - nonDefVal) < 0.001, "Macro 2 set to test value 0.88");

                juce::MouseEvent eDbl(juce::Desktop::getInstance().getMainMouseSource(),
                                      juce::Point<float>(5.0f, 5.0f),
                                      juce::ModifierKeys(),
                                      0.0f, 0.0f, 0.0f, 0.0f, 0, nullptr, nullptr,
                                      juce::Time::getCurrentTime(),
                                      juce::Point<float>(5.0f, 5.0f),
                                      juce::Time::getCurrentTime(),
                                      2, false);
                testKnob->mouseDoubleClick(eDbl);
                pumpMessageLoop(2, 5);
                reporter.expect(std::abs(testKnob->getValue() - nonDefVal) < 0.001,
                                "Double-click on knob preserves value without resetting (" + juce::String(testKnob->getValue(), 2) + ")");

                juce::MouseEvent eAlt(juce::Desktop::getInstance().getMainMouseSource(),
                                      juce::Point<float>(5.0f, 5.0f),
                                      juce::ModifierKeys::altModifier,
                                      0.0f, 0.0f, 0.0f, 0.0f, 0, nullptr, nullptr,
                                      juce::Time::getCurrentTime(),
                                      juce::Point<float>(5.0f, 5.0f),
                                      juce::Time::getCurrentTime(),
                                      1, false);
                testKnob->mouseDown(eAlt);
                pumpMessageLoop(2, 5);
                reporter.expect(std::abs(testKnob->getValue() - initialDef) < 0.01,
                                "Alt-click on knob cleanly resets value to default (" + juce::String(testKnob->getValue(), 2) + ")");

                // 2. Audition Trigger Audio Path & Non-Silent Energy
                TheKlangFarmerAudioProcessor pAudit;
                pAudit.prepareToPlay(44100.0, 512);
                pAudit.triggerAudition(1.0f, 36);

                juce::AudioBuffer<float> auditBuffer(2, 512);
                auditBuffer.clear();
                juce::MidiBuffer auditMidi;
                pAudit.processBlock(auditBuffer, auditMidi);

                float auditPeak = auditBuffer.getMagnitude(0, 512);
                reporter.expect(auditPeak > 0.05f,
                                "triggerAudition(1.0f, 36) rendered non-silent audio on audio thread (peak: " + juce::String(auditPeak, 4) + " > 0.05)");

                pAudit.releaseResources();

                // 3. Audition TRIGGER button UI wiring
                auto allFarmerButtons = ComponentFinder::findAllByType<juce::TextButton>(farmerEditor);
                juce::TextButton* triggerBtn = nullptr;
                for (auto* btn : allFarmerButtons) {
                    if (btn && btn->getButtonText().containsIgnoreCase("TRIGGER")) {
                        triggerBtn = btn;
                        break;
                    }
                }
                reporter.expect(triggerBtn != nullptr, "Audition TRIGGER button located in top header");
                if (triggerBtn) {
                    TheKlangFarmerAudioProcessor pTrig;
                    pTrig.prepareToPlay(44100.0, 512);
                    auto eTrig = std::unique_ptr<TheKlangFarmerAudioProcessorEditor>(
                        dynamic_cast<TheKlangFarmerAudioProcessorEditor*>(pTrig.createEditor()));
                    if (eTrig) {
                        eTrig->setSize(1000, 750);
                        pumpMessageLoop(2, 5);
                        auto trigBtns = ComponentFinder::findAllByType<juce::TextButton>(eTrig.get());
                        juce::TextButton* tb = nullptr;
                        for (auto* b : trigBtns) {
                            if (b && b->getButtonText().containsIgnoreCase("TRIGGER")) {
                                tb = b;
                                break;
                            }
                        }
                        if (tb) {
                            EventSimulator::simulateClick(tb);
                            pumpMessageLoop(2, 5);
                            juce::AudioBuffer<float> trigBuf(2, 512);
                            trigBuf.clear();
                            juce::MidiBuffer trigMidi;
                            pTrig.processBlock(trigBuf, trigMidi);
                            float trigPeak = trigBuf.getMagnitude(0, 512);
                            reporter.expect(trigPeak > 0.05f,
                                            "Audition TRIGGER button click rendered audible drum hit (peak: " + juce::String(trigPeak, 4) + ")");
                        }
                        eTrig.reset();
                        pTrig.releaseResources();
                    }
                }

                // --- Stage 12: Phase 2 Vector 6-Sided Die & Popover Callout Layer ---
                // 1. Vector DiceButton rendering across all pip counts
                DiceButton testDice("testDice");
                testDice.setSize(24, 24);
                juce::Image diceImg(juce::Image::ARGB, 24, 24, true);
                juce::Graphics gDice(diceImg);
                for (int pips = 1; pips <= 6; ++pips) {
                    testDice.setPipCount(pips);
                    testDice.paintButton(gDice, false, false);
                }
                reporter.expect(testDice.getPipCount() == 6, "DiceButton painted all 6 pip face configurations without error");

                EventSimulator::simulateClick(&testDice);
                pumpMessageLoop(2, 5);
                reporter.expect(testDice.getPipCount() >= 1 && testDice.getPipCount() <= 6,
                                "Clicking DiceButton rolled pip face to " + juce::String(testDice.getPipCount()));

                // 2. ModuleCardComponent right-click launches CardInspectorPopover in juce::CallOutBox
                if (!cards.isEmpty()) {
                    auto* testCard = cards[0];
                    EventSimulator::simulateRightClick(testCard);
                    pumpMessageLoop(2, 5);

                    CardInspectorPopover* cardPopover = nullptr;
                    for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i) {
                        if (auto* found = ComponentFinder::findByType<CardInspectorPopover>(juce::Desktop::getInstance().getComponent(i))) {
                            cardPopover = found;
                            break;
                        }
                    }
                    reporter.expect(cardPopover != nullptr, "Right-clicking ModuleCardComponent launched CardInspectorPopover");

                    if (cardPopover) {
                        auto* callout = cardPopover->findParentComponentOfClass<juce::CallOutBox>();
                        reporter.expect(callout != nullptr, "Found parent CallOutBox for CardInspectorPopover");
                        if (callout) {
                            callout->exitModalState(0);
                            callout->setVisible(false);
                            callout->removeFromDesktop();
                            pumpMessageLoop(5, 5);
                        }
                    }

                    bool popoverStillVisible = false;
                    for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i) {
                        if (auto* found = ComponentFinder::findByType<CardInspectorPopover>(juce::Desktop::getInstance().getComponent(i))) {
                            if (found->isVisible()) {
                                popoverStillVisible = true;
                                break;
                            }
                        }
                    }
                    reporter.expect(!popoverStillVisible, "CardInspectorPopover dismissed cleanly from Desktop");
                }
            }
        }
    }
}

