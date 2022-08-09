#include <spdlog/sinks/stdout_color_sinks.h>

#include "log.h"

std::shared_ptr<spdlog::logger> GEngine::CLog::core_logger_;
std::shared_ptr<spdlog::logger> GEngine::CLog::client_logger_;

void GEngine::CLog::Init() {
  spdlog::set_pattern("[%T] [%^%l%$] %n: %v");

  core_logger_ = spdlog::stdout_color_mt("Core");
  core_logger_->set_level(spdlog::level::trace);

  client_logger_ = spdlog::stdout_color_mt("App");
  client_logger_->set_level(spdlog::level::trace);
}
