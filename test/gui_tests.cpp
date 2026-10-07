#include "GuiTestHelpers.h"
#include "FarmerTestSuite.h"
#include "PlanterTestSuite.h"
#include "EditorTestSuite.h"
#include "SmokePaintSuite.h"
#include "ReflectionGuardrailSuite.h"
#include "HardeningSuites.h"
#include "VersionCheckerTestSuite.h"
#include "ParameterSchemaAuditTest.h"
#include "PluginIntensiveTestSuite.h"

int main(int argc, char* argv[]) {
    juce::StringArray args;
    for (int i = 1; i < argc; ++i) {
        args.add(juce::String(argv[i]));
    }

    bool runAll = args.isEmpty() || args.contains("--all");
    bool runFarmer = runAll || args.contains("--farmer");
    bool runPlanter = runAll || args.contains("--planter");
    bool runEditor = runAll || args.contains("--editor");
    bool smokeOnly = args.contains("--smoke-only");
    bool reflectionOnly = args.contains("--reflection-only");
    bool stressOnly = args.contains("--stress-only");

    GuiTestHelpers::TestReporter reporter;
    GuiTestHelpers::ScopedGuiContext guiContext;

    std::cout << "========================================" << std::endl;
    std::cout << "THE KLANG FARMER - UNIVERSAL GUI TEST HARNESS" << std::endl;
    std::cout << "========================================" << std::endl;

    if (stressOnly) {
        HardeningSuites::runSuite(reporter);
    } else if (reflectionOnly) {
        ReflectionGuardrailSuite::runSuite(reporter);
    } else if (smokeOnly) {
        SmokePaintSuite::runSuite(reporter);
    } else {
        if (runFarmer)  FarmerTestSuite::runSuite(reporter);
        if (runPlanter) PlanterTestSuite::runSuite(reporter);
        if (runEditor)  EditorTestSuite::runSuite(reporter);
        
        // Always run smoke, reflection, version checker and hardening if running all
        if (runAll) {
            VersionCheckerTestSuite::runSuite(reporter);
            SmokePaintSuite::runSuite(reporter);
            ReflectionGuardrailSuite::runSuite(reporter);
            HardeningSuites::runSuite(reporter);
            ParameterSchemaAuditTest::runSuite(reporter);
            PluginIntensiveTestSuite::runSuite(reporter);
        }
    }

    reporter.printSummary();

    VersionChecker::getInstance().stopThread(2000);
    GuiTestHelpers::pumpMessageLoop(20, 10);

    return (reporter.failed == 0) ? 0 : 1;
}
