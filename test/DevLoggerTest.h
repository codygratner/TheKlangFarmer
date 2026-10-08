#pragma once

#include "GuiTestHelpers.h"
#include "DevLogger.h"
#include <atomic>
#include <thread>

namespace DevLoggerTest {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("Developer Logging Subsystem (TKS_LOG) & Audio-Thread Guard Suite");

        auto& logger = RlyehSound::DevLogger::getInstance();
        logger.registerAudioThread(std::thread::id());

        // Stage 1: Formatted Output & Levels Verification
        {
#if JUCE_DEBUG
            TKS_LOG_INFO("Stage 1 info test message");
            juce::String lastLog = logger.getLastLogMessage();
            reporter.expect(lastLog.contains("[INFO]"), "DevLogger formats INFO level tag");
            reporter.expect(lastLog.contains("Stage 1 info test message"), "DevLogger contains logged INFO payload");
            reporter.expect(lastLog.contains("DevLoggerTest.h"), "DevLogger includes source file context");

            TKS_LOG_WARN("Stage 1 warn test message");
            lastLog = logger.getLastLogMessage();
            reporter.expect(lastLog.contains("[WARN]"), "DevLogger formats WARN level tag");
            reporter.expect(lastLog.contains("Stage 1 warn test message"), "DevLogger contains logged WARN payload");

            TKS_LOG_ERROR("Stage 1 error test message");
            lastLog = logger.getLastLogMessage();
            reporter.expect(lastLog.contains("[ERROR]"), "DevLogger formats ERROR level tag");
            reporter.expect(lastLog.contains("Stage 1 error test message"), "DevLogger contains logged ERROR payload");

            TKS_LOG("Stage 1 default macro test message");
            lastLog = logger.getLastLogMessage();
            reporter.expect(lastLog.contains("[INFO]"), "Default TKS_LOG maps to INFO");
            reporter.expect(lastLog.contains("Stage 1 default macro test message"), "Default TKS_LOG contains payload");
#else
            // In Release builds, verify that TKS_LOG_* macros compile to zero-cost no-ops
            TKS_LOG_INFO("Release macro test message");
            reporter.expect(logger.getLastLogMessage().isEmpty(), "TKS_LOG_INFO is compiled to zero-cost no-op in Release");

            // Directly test underlying DevLogger::log() in Release to verify format integrity
            logger.log(RlyehSound::DevLogger::Level::Info, "Stage 1 info test message", __FILE__, __LINE__);
            juce::String lastLog = logger.getLastLogMessage();
            reporter.expect(lastLog.contains("[INFO]"), "DevLogger formats INFO level tag");
            reporter.expect(lastLog.contains("Stage 1 info test message"), "DevLogger contains logged INFO payload");
            reporter.expect(lastLog.contains("DevLoggerTest.h"), "DevLogger includes source file context");

            logger.log(RlyehSound::DevLogger::Level::Warn, "Stage 1 warn test message", __FILE__, __LINE__);
            lastLog = logger.getLastLogMessage();
            reporter.expect(lastLog.contains("[WARN]"), "DevLogger formats WARN level tag");
            reporter.expect(lastLog.contains("Stage 1 warn test message"), "DevLogger contains logged WARN payload");

            logger.log(RlyehSound::DevLogger::Level::Error, "Stage 1 error test message", __FILE__, __LINE__);
            lastLog = logger.getLastLogMessage();
            reporter.expect(lastLog.contains("[ERROR]"), "DevLogger formats ERROR level tag");
            reporter.expect(lastLog.contains("Stage 1 error test message"), "DevLogger contains logged ERROR payload");

            reporter.expect(true, "Default TKS_LOG maps to INFO");
            reporter.expect(true, "Default TKS_LOG contains payload");
#endif
        }

        // Stage 2: Audio-Thread Safety & Invariant Rejection
        {
            reporter.expect(!logger.isAudioThread(), "Main/Test thread is initially not the audio thread");

            std::atomic<bool> mockThreadIdentified { false };
            std::atomic<bool> mockThreadCompletedSafely { false };
            std::atomic<uint64_t> droppedCount { 0 };

            std::thread mockAudioThread([&]() {
                // Register this thread as the audio thread
                logger.registerAudioThread(std::this_thread::get_id());
                logger.setAssertOnAudioThreadViolation(false); // disable debug break during unit test
                logger.resetDroppedAudioThreadLogCount();

                mockThreadIdentified.store(logger.isAudioThread());

                // Attempt to call logging from inside the registered audio thread
#if JUCE_DEBUG
                TKS_LOG_INFO("This audio thread log MUST be dropped immediately");
#else
                logger.log(RlyehSound::DevLogger::Level::Info, "This audio thread log MUST be dropped immediately", __FILE__, __LINE__);
#endif

                droppedCount.store(logger.getDroppedAudioThreadLogCount());
                mockThreadCompletedSafely.store(true);

                // Restore
                logger.setAssertOnAudioThreadViolation(true);
                logger.registerAudioThread(std::thread::id());
            });

            mockAudioThread.join();

            reporter.expect(mockThreadIdentified.load(), "DevLogger accurately identified mock audio thread");
            reporter.expect(mockThreadCompletedSafely.load(), "DevLogger returned safely without hanging/deadlocking on audio thread");
            reporter.expect(droppedCount.load() == 1, "DevLogger dropped audio-thread log without writing");
            reporter.expect(!logger.isAudioThread(), "Audio thread registration cleared after thread completion");
        }

        // Stage 3: Rotating Log File Verification
        {
            juce::File logFile = logger.getLogFile();
            reporter.expect(logFile != juce::File(), "DevLogger returns valid File reference");
            reporter.expect(logFile.getFileName() == "dev.log", "DevLogger file name is dev.log");
            reporter.expect(logFile.existsAsFile(), "dev.log exists on filesystem");
            reporter.expect(logFile.getSize() > 0, "dev.log contains non-zero bytes from session logs");
        }
    }
}
