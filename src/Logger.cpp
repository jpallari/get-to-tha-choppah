#include "Logger.h"
#include <iostream>

std::vector<LogEntry> Logger::messages;

void Logger::Log(const std::string &message) {
    std::cerr << "[LOG] " << message << '\n';
    messages.emplace_back(LOG_INFO, message);
}

void Logger::Err(const std::string &message) {
    std::cerr << "[ERR] " << message << '\n';
    messages.emplace_back(LOG_ERROR, message);
}

