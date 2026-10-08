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

                // Test theme selector & swatches
                auto& themeBox = modal->getThemeBox();
                reporter.expect(themeBox.getNumItems() >= 7, "SettingsModal themeBox has at least 7 curated themes");

                // Select Matrix Green
                int matrixItemId = -1;
                for (int i = 1; i <= themeBox.getNumItems(); ++i) {
                    if (themeBox.getItemText(i - 1).containsIgnoreCase("Matrix")) {
                        matrixItemId = themeBox.getItemId(i - 1);
                        break;
                    }
                }
                if (matrixItemId > 0) {
                    themeBox.setSelectedId(matrixItemId, juce::sendNotificationSync);
                    if (themeBox.onChange) themeBox.onChange();
                }
                pumpMessageLoop();
                auto activeTheme = RlyehSound::ParameterManager::getInstance().getActiveTheme();
                reporter.expect(activeTheme == "matrix_green",
                                "themeBox selects matrix_green theme (active: " + activeTheme + ", matrixItemId: " + juce::String(matrixItemId) + ", selectedId: " + juce::String(themeBox.getSelectedId()) + ")");
                reporter.expect(RlyehSound::ParameterManager::getInstance().getGlobalColor("accent_cyan") == juce::Colour(0xff22c55e),
                                "Matrix green theme applies 0xff22c55e accent (actual: " + RlyehSound::ParameterManager::getInstance().getGlobalColor("accent_cyan").toDisplayString(true) + ")");

                // Test Swatches
                const auto& accents = modal->getAccentSwatches();
                reporter.expect(accents.size() >= 7, "SettingsModal has 7 quick accent swatches");
                if (!accents.empty()) {
                    if (accents[0]->onClick) accents[0]->onClick();
                    pumpMessageLoop();
                    reporter.expect(RlyehSound::ParameterManager::getInstance().getGlobalColor("accent_cyan") == accents[0]->getSwatchColor(),
                                    "Clicking Cyan swatch applies Cyan accent tint");
                }

                const auto& bgs = modal->getBackgroundSwatches();
                reporter.expect(bgs.size() >= 4, "SettingsModal has 4 chassis background swatches");
                if (!bgs.empty()) {
                    if (bgs[0]->onClick) bgs[0]->onClick();
                    pumpMessageLoop();
                    reporter.expect(RlyehSound::ParameterManager::getInstance().getGlobalColor("background_dark") == bgs[0]->getSwatchColor(),
                                    "Clicking Pure Black swatch applies 0xff0a0d12 chassis tint");
                }

                // Restore default theme
                RlyehSound::ParameterManager::getInstance().loadTheme("cyberpunk");
                pumpMessageLoop();
                reporter.expect(RlyehSound::ParameterManager::getInstance().getActiveTheme() == "cyberpunk",
                                "Restored cyberpunk default theme");

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
