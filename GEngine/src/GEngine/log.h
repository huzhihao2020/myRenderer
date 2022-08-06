#pragma once

#include <memory>

#include <spdlog/spdlog.h>

namespace GEngine
{
  class CLog {
    public:
      static void Init();
      inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return core_logger_; };
      inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return client_logger_; };

    private:
      static std::shared_ptr<spdlog::logger> core_logger_;
      static std::shared_ptr<spdlog::logger> client_logger_;
  };
} // namespace GEngine

// Log Macros
#define GE_CORE_TRACE(...)	::GEngine::CLog::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...)	  ::GEngine::CLog::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...)	  ::GEngine::CLog::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...)	::GEngine::CLog::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_FATAL(...)	::GEngine::CLog::GetCoreLogger()->fatal(__VA_ARGS__GE)

#define GE_TRACE(...)	::GEngine::CLog::GetClientLogger()->trace(__VA_ARGS__)
#define GE_INFO(...)	::GEngine::CLog::GetClientLogger()->info(__VA_ARGS__)
#define GE_WARN(...)	::GEngine::CLog::GetClientLogger()->warn(__VA_ARGS__)
#define GE_ERROR(...)	::GEngine::CLog::GetClientLogger()->error(__VA_ARGS__)
#define GE_FATAL(...)	::GEngine::CLog::GetClientLogger()->fatal(__VA_ARGS__)