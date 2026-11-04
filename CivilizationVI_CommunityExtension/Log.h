#pragma once
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <iostream>
#include <filesystem>
#include <chrono>

// Simple logger implementation inspired by BepInEx
namespace Log {
    enum class Severity {
        Trace,
        Info,
        Warning,
        Error,
        Fatal
    };

    constexpr std::string_view to_string(Severity severity) noexcept {
        switch (severity) {
        case Severity::Trace:   return "Trace";
        case Severity::Info:    return "Info";
        case Severity::Warning: return "Warn";
        case Severity::Error:   return "Error";
        case Severity::Fatal:   return "Fatal";
        default:                return "Unknown";
        }
    }

    struct LogEvent {
        Severity severity;
        std::string_view tag;
        std::source_location loc;
        std::chrono::system_clock::time_point timestamp;
    };

    class LogRouter {
    private:
        inline static std::unique_ptr<LogRouter> instance = nullptr;

        std::unordered_map<std::string, std::vector<std::shared_ptr<std::ofstream>>> m_tagFiles;
        std::unordered_map<std::string, std::shared_ptr<std::ofstream>> m_openFiles;
        std::mutex m_mutex;
        std::filesystem::path m_logDirectory;

        explicit LogRouter(std::filesystem::path logDirectory)
            : m_logDirectory(std::move(logDirectory)) {}

    public:
        [[nodiscard]] static LogRouter& Get() {
            if (!instance) throw std::runtime_error("LogRouter accessed before initialization!");
            return *instance;
        }

        static void Create (std::filesystem::path logDirectory) {
            if (instance) {
                throw std::runtime_error("LogRouter is already initialized!");
            }
            // std::make_unique cannot access private constructor, so new is used here:
            instance.reset(new LogRouter(std::move(logDirectory)));
        }

        static void Destroy() {
            instance.reset();
        }

        LogRouter(const LogRouter&) = delete;
        LogRouter& operator=(const LogRouter&) = delete;

        void RegisterSink(const std::string& tag, const std::string& filename) {
            std::lock_guard lock(m_mutex);
            auto& fileStream = m_openFiles[filename];
            if (!fileStream) {
                fileStream = std::make_shared<std::ofstream>(m_logDirectory / filename, std::ios::app);
            }
            m_tagFiles[tag].push_back(fileStream);
        }

        void Route(const LogEvent& event, std::string_view message) {
            std::lock_guard lock(m_mutex);

            const char* colorCode = "";
            const char* resetCode = "\x1b[0m";

            switch (event.severity) {
            case Severity::Trace:   colorCode = "\x1b[90m"; break;
            case Severity::Warning: colorCode = "\x1b[33m"; break;
            case Severity::Error:
            case Severity::Fatal:   colorCode = "\x1b[31m"; break;
            case Severity::Info:
            default:                colorCode = "\x1b[37m"; break;
            }

            // Console output
            std::cout << std::format("{}[{}] [{}] {}{}\n",
                colorCode,
                event.tag,
                to_string(event.severity),
                message,
                resetCode);

            // File output
            if (auto it = m_tagFiles.find(std::string(event.tag)); it != m_tagFiles.end()) {
                for (auto& file : it->second) {
                    if (file && file->is_open()) {
                        *file << std::format("[{}] {}:{} - {}\n",
                            to_string(event.severity),
                            event.loc.file_name(),
                            event.loc.line(),
                            message);
                        file->flush();
                    }
                }
            }
        }
    };

    template <typename... Args>
    struct LogLocation {
        std::format_string<Args...> fmt;
        std::source_location loc;

        template <typename StringType>
        consteval LogLocation(const StringType& format_str,
            std::source_location loc = std::source_location::current())
            : fmt(format_str), loc(loc) {
        }
    };

    class Logger {
        std::string m_tag;

    public:
        explicit Logger(std::string tag, const std::vector<std::string>& filenames)
            : m_tag(std::move(tag)) {
            for (const auto& filename : filenames) {
                LogRouter::Get().RegisterSink(m_tag, filename);
            }
        }

        explicit Logger(std::string tag, std::string filename)
            : m_tag(std::move(tag)) {
            LogRouter::Get().RegisterSink(m_tag, filename);
        }

        explicit Logger(std::string tag) : m_tag(std::move(tag)) {}

        template <typename... Args>
        void Info(std::type_identity_t<LogLocation<Args...>> logLoc, Args&&... args) {
            log(Severity::Info, logLoc, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Warn(std::type_identity_t<LogLocation<Args...>> logLoc, Args&&... args) {
            log(Severity::Warning, logLoc, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Error(std::type_identity_t<LogLocation<Args...>> logLoc, Args&&... args) {
            log(Severity::Error, logLoc, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Trace(std::type_identity_t<LogLocation<Args...>> logLoc, Args&&... args) {
            log(Severity::Trace, logLoc, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Fatal(std::type_identity_t<LogLocation<Args...>> logLoc, Args&&... args) {
            log(Severity::Fatal, logLoc, std::forward<Args>(args)...);
        }

    private:
        template <typename... Args>
        void log(Severity severity, LogLocation<Args...> logLoc, Args&&... args) {
            std::string message = std::format(logLoc.fmt, std::forward<Args>(args)...);

            LogEvent event{
                .severity = severity,
                .tag = m_tag,
                .loc = logLoc.loc,
                .timestamp = std::chrono::system_clock::now()
            };

            LogRouter::Get().Route(event, message);
        }
    };
}