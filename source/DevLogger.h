#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <memory>
#include <thread>

namespace RlyehSound {

class DevLogger {
public:
    enum class Level {
        Info,
        Warn,
        Error
    };

    static DevLogger& getInstance() {
        static DevLogger instance;
        return instance;
    }

    void registerAudioThread(std::thread::id id) noexcept {
        audioThreadId.store(id, std::memory_order_release);
    }

    bool isAudioThread() const noexcept {
        const auto registeredId = audioThreadId.load(std::memory_order_acquire);
        return registeredId != std::thread::id() && std::this_thread::get_id() == registeredId;
    }

    void setAssertOnAudioThreadViolation(bool shouldAssert) noexcept {
        assertOnAudioThread = shouldAssert;
    }

    uint64_t getDroppedAudioThreadLogCount() const noexcept {
        return droppedAudioThreadLogs.load(std::memory_order_relaxed);
    }

    void resetDroppedAudioThreadLogCount() noexcept {
        droppedAudioThreadLogs.store(0, std::memory_order_relaxed);
    }

    void log(Level level, const juce::String& message, const char* file = nullptr, int line = 0) {
        if (isAudioThread()) {
            droppedAudioThreadLogs.fetch_add(1, std::memory_order_relaxed);
            if (assertOnAudioThread) {
                jassert(!isAudioThread());
            }
            return;
        }

        const char* levelStr = "INFO";
        switch (level) {
            case Level::Info:  levelStr = "INFO";  break;
            case Level::Warn:  levelStr = "WARN";  break;
            case Level::Error: levelStr = "ERROR"; break;
        }

        juce::String formatted;
        formatted << "[" << juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S.%ms") << "] "
                  << "[" << levelStr << "] ";

        if (file != nullptr && file[0] != '\0') {
            juce::String path(file);
            int lastSlash = path.lastIndexOfAnyOf("/\\");
            juce::String filename = (lastSlash >= 0) ? path.substring(lastSlash + 1) : path;
            formatted << "[" << filename << ":" << line << "] ";
        }

        formatted << message;

        const juce::ScopedLock sl(lock);
        lastLogMessage = formatted;

        if (fileLogger != nullptr) {
            fileLogger->logMessage(formatted);
        } else {
            DBG(formatted);
        }
    }

    juce::File getLogFile() const {
        const juce::ScopedLock sl(lock);
        if (fileLogger != nullptr)
            return fileLogger->getLogFile();
        return {};
    }

    juce::String getLastLogMessage() const {
        const juce::ScopedLock sl(lock);
        return lastLogMessage;
    }

private:
    DevLogger() {
        fileLogger.reset(juce::FileLogger::createDefaultAppLogger(
            "TheKlangSuite",
            "dev.log",
            "=== The Klang Suite Dev Session ===",
            5 * 1024 * 1024));
    }

    ~DevLogger() = default;
    DevLogger(const DevLogger&) = delete;
    DevLogger& operator=(const DevLogger&) = delete;

    std::atomic<std::thread::id> audioThreadId { std::thread::id() };
    std::atomic<uint64_t> droppedAudioThreadLogs { 0 };
    bool assertOnAudioThread { true };

    mutable juce::CriticalSection lock;
    std::unique_ptr<juce::FileLogger> fileLogger;
    juce::String lastLogMessage;
};

} // namespace RlyehSound

#if JUCE_DEBUG
  #define TKS_LOG_INFO(msg)  ::RlyehSound::DevLogger::getInstance().log(::RlyehSound::DevLogger::Level::Info,  (msg), __FILE__, __LINE__)
  #define TKS_LOG_WARN(msg)  ::RlyehSound::DevLogger::getInstance().log(::RlyehSound::DevLogger::Level::Warn,  (msg), __FILE__, __LINE__)
  #define TKS_LOG_ERROR(msg) ::RlyehSound::DevLogger::getInstance().log(::RlyehSound::DevLogger::Level::Error, (msg), __FILE__, __LINE__)
  #define TKS_LOG(msg)       TKS_LOG_INFO(msg)
#else
  #define TKS_LOG_INFO(msg)  do {} while (false)
  #define TKS_LOG_WARN(msg)  do {} while (false)
  #define TKS_LOG_ERROR(msg) do {} while (false)
  #define TKS_LOG(msg)       do {} while (false)
#endif
