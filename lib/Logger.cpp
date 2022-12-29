#include "Logger.h"

using namespace thisptr::broker;
std::shared_ptr<spdlog::logger> thisptr::broker::Logger::m_logger = nullptr;

void Logger::init() {
  if (m_logger)
    return;
  m_logger = spdlog::stdout_color_mt("logger");
  m_logger->set_level(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger> Logger::logger() {
  return m_logger;
}
