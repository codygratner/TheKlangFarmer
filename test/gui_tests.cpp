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
#include "DevLoggerTest.h"
#include "ChaosMonkeySuite.h"

int main(int argc, char* argv[]) {
    juce::StringArray args;
    for (int i = 1; i < argc; ++i) {
        args.add(juce::String(argv[i]));
    }

    bool runChaos = args.contains("--chaos");
    uint32_t chaosSeed = 0;
    for (const auto& arg : args) {
        if (arg.startsWith("--seed=")) {
            chaosSeed = static_cast<uint32_t>(arg.substring(7).getLargeIntValue());
        }
    }

    bool runAll = (args.isEmpty() || args.contains("--all")) && !runChaos;
    bool runFarmer = runAll || args.contains("--farmer");
    bool runPlanter = runAll || args.contains("--planter");
    bool runEditor = runAll || args.contains("--editor");
    bool smokeOnly = args.contains("--smoke-only");
    bool reflectionOnly = args.contains("--reflection-only");
    bool stressOnly = args.contains("--stress-only");
    bool versionOnly = args.contains("--version-only");
    bool auditOnly = args.contains("--audit-only");
    bool intensiveOnly = args.contains("--intensive-only");
    bool loggerOnly = args.contains("--logger-only");

    // Initialize Watchdog: 5m global timeout, 30s step heartbeat limit
    GuiTestHelpers::Watchdog::start(5, 30);

    GuiTestHelpers::TestReporter reporter;
    GuiTestHelpers::ScopedGuiContext guiContext;

    std::cout << "========================================" << std::endl;
    std::cout << "THE KLANG FARMER - UNIVERSAL GUI TEST HARNESS" << std::endl;
    std::cout << "========================================" << std::endl;

    if (runChaos) {
        ChaosMonkeySuite::runSuite(reporter, chaosSeed);
    } else if (stressOnly) {
        HardeningSuites::runSuite(reporter);
    } else if (reflectionOnly) {
        ReflectionGuardrailSuite::runSuite(reporter);
    } else if (smokeOnly) {
        SmokePaintSuite::runSuite(reporter);
    } else if (versionOnly) {
        VersionCheckerTestSuite::runSuite(reporter);
    } else if (auditOnly) {
        ParameterSchemaAuditTest::runSuite(reporter);
    } else if (intensiveOnly) {
        PluginIntensiveTestSuite::runSuite(reporter);
    } else if (loggerOnly) {
        DevLoggerTest::runSuite(reporter);
    } else {
        if (runFarmer)  FarmerTestSuite::runSuite(reporter);
        if (runPlanter) PlanterTestSuite::runSuite(reporter);
        if (runEditor)  EditorTestSuite::runSuite(reporter);
        
        // Always run smoke, reflection, version checker and hardening if running all
        if (runAll) {
            DevLoggerTest::runSuite(reporter);
            VersionCheckerTestSuite::runSuite(reporter);
            SmokePaintSuite::runSuite(reporter);
            ReflectionGuardrailSuite::runSuite(reporter);
            HardeningSuites::runSuite(reporter);
            ParameterSchemaAuditTest::runSuite(reporter);
            PluginIntensiveTestSuite::runSuite(reporter);
        }
    }

    if (GuiTestHelpers::Watchdog::testFailedDueToTimeout.load()) {
        reporter.expect(false, "One or more test steps timed out waiting for heartbeat");
    }

    reporter.printSummary();
    std::cout.flush();
    std::cerr.flush();

    VersionChecker::teardown();
    GuiTestHelpers::pumpMessageLoop(20, 10);

    GuiTestHelpers::Watchdog::stop();

    return (reporter.failed == 0) ? 0 : 1;
}
