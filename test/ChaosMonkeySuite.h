#pragma once

#include "GuiTestHelpers.h"
#include "FarmerProcessor.h"
#include "FarmerEditor.h"
#include "PlanterProcessor.h"
#include "PlanterEditor.h"
#include <random>

namespace ChaosMonkeySuite {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter, uint32_t seed = 0, int durationMs = 3000) {
        reporter.beginTest("Seeded & Replayable UI Chaos Monkey Suite");

        if (seed == 0) {
            std::random_device rd;
            seed = rd();
        }

        std::cout << "🐒 [CHAOS MONKEY] Execution Seed: " << seed << std::endl;
        std::cout << "🐒 [CHAOS MONKEY] Reproduction CLI: gui_tests --chaos --seed=" << seed << std::endl;

        juce::Random rng(static_cast<juce::int64>(seed));

        TheKlangFarmerAudioProcessor farmerProcessor;
        auto farmerEditor = std::unique_ptr<juce::AudioProcessorEditor>(farmerProcessor.createEditor());
        farmerEditor->setSize(1000, 750);

        TheKlangPlanterAudioProcessor planterProcessor;
        auto planterEditor = std::unique_ptr<juce::AudioProcessorEditor>(planterProcessor.createEditor());
        planterEditor->setSize(1000, 750);

        pumpMessageLoop(10, 10);

        auto startTime = juce::Time::getMillisecondCounter();
        int eventCount = 0;

        try {
            while (juce::Time::getMillisecondCounter() - startTime < static_cast<juce::uint32>(durationMs)) {
                Watchdog::pingHeartbeat();

                // Select target editor randomly
                juce::Component* targetEditor = (rng.nextBool() ? static_cast<juce::Component*>(farmerEditor.get()) : static_cast<juce::Component*>(planterEditor.get()));

                // Random event type:
                // 0: Click random child component
                // 1: Drag random child component
                // 2: Double click random child component
                // 3: Resize window
                int action = rng.nextInt(4);

                auto children = targetEditor->getChildren();
                if (action == 3 || children.isEmpty()) {
                    int newW = rng.nextInt(juce::Range<int>(600, 1400));
                    int newH = rng.nextInt(juce::Range<int>(500, 1000));
                    targetEditor->setSize(newW, newH);
                } else {
                    // Pick a random child component (drilling down)
                    juce::Component* comp = children[rng.nextInt(children.size())];
                    while (comp != nullptr && !comp->getChildren().isEmpty() && rng.nextFloat() > 0.3f) {
                        auto sub = comp->getChildren();
                        comp = sub[rng.nextInt(sub.size())];
                    }

                    if (comp != nullptr && comp->isVisible()) {
                        if (action == 0) {
                            EventSimulator::simulateClick(comp);
                        } else if (action == 1) {
                            float dragY = rng.nextFloat() * 100.0f - 50.0f;
                            EventSimulator::simulateDrag(comp, dragY);
                        } else if (action == 2) {
                            EventSimulator::simulateDoubleClick(comp);
                        }
                    }
                }

                eventCount++;
                pumpMessageLoop(1, 5);
            }

            reporter.expect(eventCount > 0, "Chaos monkey survived " + juce::String(eventCount) + " randomized events (Seed: " + juce::String(seed) + ")");
        } catch (const std::exception& ex) {
            std::cerr << "💥 [CHAOS MONKEY CRASH]: Exception caught: " << ex.what() << std::endl;
            std::cerr << "💥 Reproduce with: gui_tests --chaos --seed=" << seed << std::endl;
            reporter.expect(false, "Chaos monkey crashed with seed " + juce::String(seed) + ": " + juce::String(ex.what()));
        } catch (...) {
            std::cerr << "💥 [CHAOS MONKEY CRASH]: Unknown exception caught!" << std::endl;
            std::cerr << "💥 Reproduce with: gui_tests --chaos --seed=" << seed << std::endl;
            reporter.expect(false, "Chaos monkey crashed with unknown exception, seed: " + juce::String(seed));
        }

        std::cout << "🐒 [CHAOS MONKEY COMPLETE]: " << eventCount << " events delivered in " << durationMs << "ms." << std::endl;
    }
}
