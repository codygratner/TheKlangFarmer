#include "GuiTestHelpers.h"
#include "FarmerTestSuite.h"
#include "PlanterTestSuite.h"
#include "EditorTestSuite.h"
#include "SmokePaintSuite.h"
#include "ReflectionGuardrailSuite.h"
#include "HardeningSuites.h"

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
        
        // Always run smoke, reflection and hardening if running all
        if (runAll) {
            SmokePaintSuite::runSuite(reporter);
            ReflectionGuardrailSuite::runSuite(reporter);
            HardeningSuites::runSuite(reporter);
        }
    }

    reporter.printSummary();

    return (reporter.failed == 0) ? 0 : 1;
}
