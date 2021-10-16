#pragma once

#pragma warning(push, 0)
#pragma comment(lib, "spdlogd.lib")

#include "spdlog/spdlog.h"
#include "spdlog/sinks/msvc_sink.h"
#pragma warning(pop)

inline auto InitLogger() {
    auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("msvc_logger", sink);
    spdlog::initialize_logger(logger);

    spdlog::set_default_logger(logger);

    if constexpr(_DEBUG)
        spdlog::set_level(spdlog::level::debug);
}

inline auto Log() {
    spdlog::info("Welcome to spdlog!");
    spdlog::debug("Debug only msg");
    //spdlog::get("msvc_logger")->info("Welcome to spdlog!");
    // spdlog::error("Some error message with arg: {}", 1);
    // spdlog::warn("Easy padding in numbers like {:08d}", 12);
    // spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
}