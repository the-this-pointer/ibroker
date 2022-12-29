#ifndef IBROKER_LOGGER_H
#define IBROKER_LOGGER_H

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace thisptr {
  namespace broker {

    class Logger {
    public:
      static void init();
      static std::shared_ptr<spdlog::logger> logger();
    private:
      static std::shared_ptr<spdlog::logger> m_logger;
    };

  }
}

#ifdef ENABLE_LOGGER
#define LT(...) ::thisptr::broker::Logger::logger()->trace(__VA_ARGS__)
#define LD(...) ::thisptr::broker::Logger::logger()->debug(__VA_ARGS__)
#define LI(...) ::thisptr::broker::Logger::logger()->info(__VA_ARGS__)
#define LW(...) ::thisptr::broker::Logger::logger()->warn(__VA_ARGS__)
#define LE(...) ::thisptr::broker::Logger::logger()->error(__VA_ARGS__)
#define LC(...) ::thisptr::broker::Logger::logger()->critical(__VA_ARGS__)
#else
#define LT(...)
#define LD(...)
#define LI(...)
#define LW(...)
#define LE(...)
#define LC(...)
#endif

#endif //IBROKER_LOGGER_H
