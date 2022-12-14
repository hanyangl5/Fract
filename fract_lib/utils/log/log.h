/*****************************************************************//**
 * \file   Log.h
 * \brief  log functions
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

#include <Windows.h>
#include <spdlog/spdlog.h>

#include "../singleton/public_singleton.h"
#include "../container/Container.h"

namespace Fract {

inline Container::String HrToString(HRESULT hr) {
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<unsigned int>(hr));
    return Container::String(s_str);
}

class Log : public PublicSingleton<Log> {
  public:
    enum loglevel { debug, info, warn, error, fatal };

  public:
    Log() noexcept;
    ~Log() noexcept;

    template <typename... args> inline void Debug(args &&..._args) const noexcept {
        m_logger->debug(std::forward<args>(_args)...);
    }

    template <typename... args> inline void Info(args &&..._args) const noexcept {
        m_logger->info(std::forward<args>(_args)...);
    }

    template <typename... args> inline void Warn(args &&..._args) const noexcept {
        m_logger->warn(std::forward<args>(_args)...);
    }

    template <typename... args> inline void Error(args &&..._args) const noexcept {
        m_logger->error(std::forward<args>(_args)...);
    }
    template <typename... args> inline void Fatal(args &&..._args) const noexcept {
        // m_logger->fatal(std::forward<args>(_args)...);
    }
    void CheckDXResult(HRESULT hr, const char *func_name, int line) const noexcept;

  private:
    std::shared_ptr<spdlog::logger> m_logger;
};

#define LOG_DEBUG(...) Log::GetInstance().Debug("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_INFO(...) Log::GetInstance().Info("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_WARN(...) Log::GetInstance().Warn("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_ERROR(...) Log::GetInstance().Error("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_FATAL(...) Log::GetInstance().Fatal("[" + Container::String(__FUNCTION__) + "] " + __VA_ARGS__);

#define CHECK_VK_RESULT(res) Log::GetInstance().CheckVulkanResult(res, __FUNCTION__, __LINE__);

#define CHECK_DX_RESULT(res) Log::GetInstance().CheckDXResult(res, __FUNCTION__, __LINE__);

} // namespace Fract
