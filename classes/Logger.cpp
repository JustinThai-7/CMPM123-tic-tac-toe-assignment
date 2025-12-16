#include "Logger.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace ClassGame {

Logger &Logger::GetInstance() {
  static Logger instance;
  return instance;
}

Logger::Logger() : m_fileLoggingEnabled(false) {
  // Initialize with default log file
  SetLogFile("application.log");
}

Logger::~Logger() {
  if (m_logFile.is_open()) {
    m_logFile.close();
  }
}

void Logger::LogInfo(const std::string &message) {
  AddLogEntry(LogLevel::INFO, message);
}

void Logger::LogWarning(const std::string &message) {
  AddLogEntry(LogLevel::WARNING, message);
}

void Logger::LogError(const std::string &message) {
  AddLogEntry(LogLevel::ERROR, message);
}

void Logger::LogDebug(const std::string &message) {
  AddLogEntry(LogLevel::DEBUG, message);
}

void Logger::LogGameEvent(const std::string &message) {
  AddLogEntry(LogLevel::GAME_EVENT, message);
}

void Logger::SetLogFile(const std::string &filename) {
  std::lock_guard<std::mutex> lock(m_logMutex);

  if (m_logFile.is_open()) {
    m_logFile.close();
  }

  m_logFilename = filename;
  m_logFile.open(filename, std::ios::app);

  if (!m_logFile.is_open()) {
    std::cerr << "Failed to open log file: " << filename << std::endl;
    m_fileLoggingEnabled = false;
  } else {
    m_fileLoggingEnabled = true;
    // Write header to log file
    m_logFile << "\n=== Log Session Started: " << GetCurrentTimestamp()
              << " ===\n";
    m_logFile.flush();
  }
}

void Logger::EnableFileLogging(bool enable) {
  std::lock_guard<std::mutex> lock(m_logMutex);
  m_fileLoggingEnabled = enable && m_logFile.is_open();
}

void Logger::AddLogEntry(LogLevel level, const std::string &message) {
  std::lock_guard<std::mutex> lock(m_logMutex);

  std::string timestamp = GetCurrentTimestamp();
  LogEntry entry(level, message, timestamp);

  // Add to memory for ImGui display
  m_logEntries.push_back(entry);

  // Limit memory usage
  if (m_logEntries.size() > MAX_LOG_ENTRIES) {
    m_logEntries.erase(m_logEntries.begin(),
                       m_logEntries.begin() +
                           (m_logEntries.size() - MAX_LOG_ENTRIES));
  }

  // Write to file if enabled
  if (m_fileLoggingEnabled) {
    WriteToFile(entry);
  }

  // Also output to console for debugging
  std::cout << "[" << GetLogLevelString(level) << "] " << timestamp << ": "
            << message << std::endl;
}

void Logger::WriteToFile(const LogEntry &entry) {
  if (!m_logFile.is_open())
    return;

  m_logFile << "[" << GetLogLevelString(entry.level) << "] " << entry.timestamp
            << ": " << entry.message << std::endl;
  m_logFile.flush();
}

const std::vector<LogEntry> &Logger::GetLogEntries() const {
  return m_logEntries;
}

void Logger::ClearLogs() {
  std::lock_guard<std::mutex> lock(m_logMutex);
  m_logEntries.clear();
}

std::string Logger::GetLogLevelString(LogLevel level) const {
  switch (level) {
  case LogLevel::INFO:
    return "INFO";
  case LogLevel::WARNING:
    return "WARN";
  case LogLevel::ERROR:
    return "ERROR";
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::GAME_EVENT:
    return "GAME";
  default:
    return "UNKNOWN";
  }
}

std::string Logger::GetCurrentTimestamp() const {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

  return ss.str();
}

} // namespace ClassGame
