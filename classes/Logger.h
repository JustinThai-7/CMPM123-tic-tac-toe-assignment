#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ClassGame {

enum class LogLevel { INFO, WARNING, ERROR, DEBUG, GAME_EVENT };

struct LogEntry {
  LogLevel level;
  std::string message;
  std::string timestamp;

  LogEntry(LogLevel l, const std::string &msg, const std::string &ts)
      : level(l), message(msg), timestamp(ts) {}
};

class Logger {
public:
  // Singleton pattern
  static Logger &GetInstance();

  // Disable copy constructor and assignment operator
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  // Destructor
  ~Logger();

  // Logging methods
  void LogInfo(const std::string &message);
  void LogWarning(const std::string &message);
  void LogError(const std::string &message);
  void LogDebug(const std::string &message);
  void LogGameEvent(const std::string &message);

  // File logging control
  void SetLogFile(const std::string &filename);
  void EnableFileLogging(bool enable);

  // ImGui console access
  const std::vector<LogEntry> &GetLogEntries() const;
  void ClearLogs();

  // Utility
  std::string GetLogLevelString(LogLevel level) const;
  std::string GetCurrentTimestamp() const;

private:
  Logger();

  void AddLogEntry(LogLevel level, const std::string &message);
  void WriteToFile(const LogEntry &entry);

  std::vector<LogEntry> m_logEntries;
  std::ofstream m_logFile;
  std::string m_logFilename;
  bool m_fileLoggingEnabled;
  std::mutex m_logMutex;

  // Maximum number of log entries to keep in memory (for ImGui display)
  static const size_t MAX_LOG_ENTRIES = 1000;
};

} // namespace ClassGame
