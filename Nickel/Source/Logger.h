#pragma once

#pragma comment(lib, "spdlogd.lib")

#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#pragma warning(pop)

namespace Nickel {
    static class Logger {
    public:
        static void Init();

        template <typename T>
        static auto Debug(const T& val) -> void {
            GetMainLogger()->debug(val);
        }

        template <typename T>
        static auto Trace(const T& val) -> void {
            GetMainLogger()->trace(val);
        }

        template <typename T>
        static auto Info(const T& val) -> void {
            GetMainLogger()->info(val);
        }

        template <typename T>
        static auto Warn(const T& val) -> void {
            GetMainLogger()->warn(val);
        }

        template <typename T>
        static auto Error(const T& val) -> void {
            GetMainLogger()->error(val);
        }

        template <typename T>
        static auto Critical(const T& val) -> void {
            GetMainLogger()->critical(val);
        }
        
        static std::shared_ptr<spdlog::logger>& GetMainLogger() { return _mainLogger; }

    private:
        static std::shared_ptr<spdlog::logger> _mainLogger;
    };
}