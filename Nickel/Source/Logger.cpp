#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/sinks/msvc_sink.h"

namespace Nickel {
    std::shared_ptr<spdlog::logger> Logger::_mainLogger;

	void Logger::Init() {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Nickel.log", true));
        sinks.emplace_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());

        sinks[0]->set_pattern("%^[%T] %n: %v%$");
        sinks[1]->set_pattern("[%T] [%l] %n: %v");
        sinks[2]->set_pattern("%^[%T] %n: %v%$");

        _mainLogger = std::make_unique<spdlog::logger>("msvc_logger", begin(sinks), end(sinks));
        spdlog::initialize_logger(_mainLogger);
        spdlog::set_default_logger(_mainLogger);

        if constexpr(_DEBUG)
            spdlog::set_level(spdlog::level::debug);
	}
}