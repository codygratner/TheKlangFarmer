#pragma once

#include "GuiTestHelpers.h"
#include "FarmerProcessor.h"
#include "FarmerEditor.h"
#include "PlanterProcessor.h"
#include "PlanterEditor.h"
#include "VersionChecker.h"
#include "SettingsModal.h"

namespace VersionCheckerTestSuite {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("GitHub Release Version Checker & Settings Modal Suite");

        // 1. Semantic Version String Comparison
        reporter.expect(VersionChecker::compareVersionStrings("0.3.0", "v0.2.0") > 0, "0.3.0 > v0.2.0");
        reporter.expect(VersionChecker::compareVersionStrings("0.3.0", "0.3.0") == 0, "0.3.0 == 0.3.0");
        reporter.expect(VersionChecker::compareVersionStrings("0.3.0", "v0.4.0") < 0, "0.3.0 < v0.4.0 (Update available)");
        reporter.expect(VersionChecker::compareVersionStrings("0.3.0", "v1.0.0") < 0, "0.3.0 < v1.0.0 (Major update)");
        reporter.expect(VersionChecker::compareVersionStrings("0.3.0", "0.3.0-rc1") > 0, "0.3.0 final > 0.3.0-rc1 prerelease");

        // 2. Preferences Persistence
        bool originalSetting = VersionChecker::isCheckOnLaunchEnabled();
        VersionChecker::setCheckOnLaunchEnabled(false);
        reporter.expect(!VersionChecker::isCheckOnLaunchEnabled(), "Disabled check on launch saved to preferences");
        VersionChecker::setCheckOnLaunchEnabled(true);
        reporter.expect(VersionChecker::isCheckOnLaunchEnabled(), "Enabled check on launch saved to preferences");
        VersionChecker::setCheckOnLaunchEnabled(originalSetting);

        // 3. Farmer Editor Settings Button & Modal Interaction
        {
            TheKlangFarmerAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            editor->setSize(1000, 750);
            pumpMessageLoop();

            auto* gearBtn = ComponentFinder::findByType<GearButton>(editor.get());
            reporter.expect(gearBtn != nullptr, "Farmer has GearButton in header");

            auto* modal = ComponentFinder::findByType<SettingsModalComponent>(editor.get());
            reporter.expect(modal != nullptr, "Farmer has SettingsModalComponent");
            if (modal) {
                reporter.expect(!modal->isVisible(), "SettingsModal initially hidden");
            }

            if (gearBtn && modal) {
                if (gearBtn->onClick) gearBtn->onClick();
                pumpMessageLoop();
                reporter.expect(modal->isVisible(), "Clicking GearButton opens SettingsModal");

                // Test closing via escape
                modal->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
                pumpMessageLoop();
                reporter.expect(!modal->isVisible(), "Escape key closes SettingsModal");
            }

            // 4. Update Badge Mocking & Visibility
            auto* badgeBtn = ComponentFinder::findByType<UpdateBadgeButton>(editor.get());
            reporter.expect(badgeBtn != nullptr, "Farmer has UpdateBadgeButton in header");

            // Mock an update available: v9.0.0
            VersionChecker::getInstance().setMockRelease("v9.0.0", "https://github.com/codygratner/TheKlangSuite/releases/tag/v9.0.0");
            pumpMessageLoop();

            if (badgeBtn) {
                reporter.expect(badgeBtn->isVisible(), "UpdateBadgeButton is visible when newer version is found");
                reporter.expect(badgeBtn->getButtonText().contains("v9.0.0"), "Update badge displays candidate tag v9.0.0");
            }

            // Mock up to date
            VersionChecker::getInstance().setMockRelease("v0.2.0", "https://github.com/codygratner/TheKlangSuite/releases/tag/v0.2.0");
            pumpMessageLoop();

            if (badgeBtn) {
                reporter.expect(!badgeBtn->isVisible(), "UpdateBadgeButton is hidden when up to date");
            }

            VersionChecker::getInstance().clearMockRelease();
            pumpMessageLoop();
        }

        // 5. Planter Editor Settings Button & Modal Interaction
        {
            TheKlangPlanterAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            editor->setSize(1040, 740);
            pumpMessageLoop();

            auto* gearBtn = ComponentFinder::findByType<GearButton>(editor.get());
            reporter.expect(gearBtn != nullptr, "Planter has GearButton in header");

            auto* modal = ComponentFinder::findByType<SettingsModalComponent>(editor.get());
            reporter.expect(modal != nullptr, "Planter has SettingsModalComponent");

            auto* badgeBtn = ComponentFinder::findByType<UpdateBadgeButton>(editor.get());
            reporter.expect(badgeBtn != nullptr, "Planter has UpdateBadgeButton in header");

            if (gearBtn && modal) {
                if (gearBtn->onClick) gearBtn->onClick();
                pumpMessageLoop();
                reporter.expect(modal->isVisible(), "Clicking GearButton in Planter opens SettingsModal");

                modal->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
                pumpMessageLoop();
                reporter.expect(!modal->isVisible(), "Escape key closes Planter SettingsModal");
            }
        }

        VersionChecker::getInstance().stopThread(2000);
        pumpMessageLoop();
    }
}
