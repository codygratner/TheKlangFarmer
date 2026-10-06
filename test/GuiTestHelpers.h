#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_events/juce_events.h>
#include <iostream>
#include "UIComponents.h"

namespace GuiTestHelpers {

    // ==============================================================================
    // 1. Headless GUI Environment Setup
    // ==============================================================================
    struct ScopedGuiContext {
        juce::ScopedJuceInitialiser_GUI guiInitialiser;
        ScopedGuiContext() {}
    };

    // ==============================================================================
    // 2. Micro Message Pump
    // ==============================================================================
    inline void pumpMessageLoop(int maxIterations = 20, int timeoutMs = 10) {
        for (int i = 0; i < maxIterations; ++i) {
            juce::Timer::callAfterDelay(timeoutMs, []{ juce::MessageManager::getInstance()->stopDispatchLoop(); });
            juce::MessageManager::getInstance()->runDispatchLoop();
        }
    }

    // ==============================================================================
    // 3. Component Finder
    // ==============================================================================
    struct ComponentFinder {
        
        template <typename T>
        static T* findByType(juce::Component* parent) {
            if (!parent) return nullptr;
            if (auto* typed = dynamic_cast<T*>(parent))
                return typed;

            for (auto* child : parent->getChildren()) {
                if (auto* found = findByType<T>(child))
                    return found;
            }
            return nullptr;
        }

        template <typename T>
        static juce::Array<T*> findAllByType(juce::Component* parent) {
            juce::Array<T*> results;
            if (!parent) return results;

            if (auto* typed = dynamic_cast<T*>(parent))
                results.add(typed);

            for (auto* child : parent->getChildren()) {
                results.addArray(findAllByType<T>(child));
            }
            return results;
        }

        static juce::Component* findByName(juce::Component* parent, const juce::String& name) {
            if (!parent) return nullptr;
            if (parent->getName() == name)
                return parent;

            for (auto* child : parent->getChildren()) {
                if (auto* found = findByName(child, name))
                    return found;
            }
            return nullptr;
        }

        static RotaryKnobSlider* findSliderByParamId(juce::Component* parent, const juce::String& paramId) {
            if (!parent) return nullptr;
            
            if (auto* slider = dynamic_cast<RotaryKnobSlider*>(parent)) {
                if (slider->getParamId() == paramId)
                    return slider;
            }

            for (auto* child : parent->getChildren()) {
                if (auto* found = findSliderByParamId(child, paramId))
                    return found;
            }
            return nullptr;
        }
        
        static LedSelectorComponent* findSelectorByParamId(juce::Component* parent, const juce::String& paramId) {
            if (!parent) return nullptr;
            
            if (auto* selector = dynamic_cast<LedSelectorComponent*>(parent)) {
                if (selector->getParamId() == paramId)
                    return selector;
            }

            for (auto* child : parent->getChildren()) {
                if (auto* found = findSelectorByParamId(child, paramId))
                    return found;
            }
            return nullptr;
        }
    };

    // ==============================================================================
    // 4. Event Simulator
    // ==============================================================================
    struct EventSimulator {
        
        static void simulateClick(juce::Component* comp) {
            if (!comp) return;
            juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);

            comp->mouseDown(e);
            juce::MouseEvent eUp(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);
            comp->mouseUp(eUp);
        }
        
        static void simulateRightClick(juce::Component* comp) {
            if (!comp) return;
            juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys::rightButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);

            comp->mouseDown(e);
            juce::MouseEvent eUp(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);
            comp->mouseUp(eUp);
        }

        static void simulateDoubleClick(juce::Component* comp) {
            if (!comp) return;
            juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 2, false);

            comp->mouseDoubleClick(e);
        }

        static void simulateDrag(juce::Component* comp, float distanceY = -50.0f) {
            if (!comp) return;
            
            juce::Point<float> startPos = comp->getLocalBounds().getCentre().toFloat();
            juce::Point<float> endPos = startPos.translated(0.0f, distanceY);
            
            juce::MouseEvent eDown(juce::Desktop::getInstance().getMainMouseSource(), startPos, juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), startPos, juce::Time::getCurrentTime(), 1, false);
            comp->mouseDown(eDown);
            
            juce::MouseEvent eDrag(juce::Desktop::getInstance().getMainMouseSource(), endPos, juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), startPos, juce::Time::getCurrentTime(), 1, true);
            comp->mouseDrag(eDrag);
            
            juce::MouseEvent eUp(juce::Desktop::getInstance().getMainMouseSource(), endPos, juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), startPos, juce::Time::getCurrentTime(), 1, true);
            comp->mouseUp(eUp);
        }
    };

    // 5. Failure Capture
    // ==============================================================================
    inline void captureFailureArtifact(juce::Component& comp, const juce::String& testName, const juce::String& stepName) {
        auto img = comp.createComponentSnapshot(comp.getLocalBounds());
        juce::File dir("test_artifacts/gui");
        if (!dir.exists()) dir.createDirectory();
        
        auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
        juce::File file = dir.getChildFile(testName + "_" + stepName + "_" + timestamp + ".png");
        juce::FileOutputStream stream(file);
        if (stream.openedOk()) {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            std::cerr << "  [SNAPSHOT SAVED]: " << file.getFullPathName() << std::endl;
        }
    }

    // ==============================================================================
    // 6. Test Reporter
    // ==============================================================================
    struct TestReporter {
        int passed = 0;
        int failed = 0;
        juce::String currentTest;

        void beginTest(const juce::String& testName) {
            currentTest = testName;
            std::cout << "\n[TEST] " << testName << std::endl;
        }

        bool expect(bool condition, const juce::String& message, juce::Component* compToSnapshot = nullptr, const juce::String& stepName = "") {
            if (condition) {
                passed++;
                return true;
            } else {
                failed++;
                std::cerr << "  [FAILED] " << message << std::endl;
                if (compToSnapshot) {
                    captureFailureArtifact(*compToSnapshot, currentTest, stepName);
                }
                return false;
            }
        }

        void printSummary() {
            std::cout << "\n========================================" << std::endl;
            std::cout << "GUI TEST SUMMARY" << std::endl;
            std::cout << "Passed: " << passed << std::endl;
            std::cout << "Failed: " << failed << std::endl;
            std::cout << "========================================" << std::endl;
            if (failed > 0) {
                std::cerr << "SOME TESTS FAILED." << std::endl;
            } else {
                std::cout << "ALL TESTS PASSED SUCCESSFULLY." << std::endl;
            }
        }
    };

} // namespace GuiTestHelpers
