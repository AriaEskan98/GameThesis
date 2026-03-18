#include "gepch.h"
#include "Engine/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace GameEngine {

	Handle<spdlog::logger> Log::gsCoreLogger;
	Handle<spdlog::logger> Log::gsClientLogger;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("GameEngine.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		gsCoreLogger = std::make_shared<spdlog::logger>("GAMEENGINE", begin(logSinks), end(logSinks));
		spdlog::register_logger(gsCoreLogger);
		gsCoreLogger->set_level(spdlog::level::trace);
		gsCoreLogger->flush_on(spdlog::level::trace);

		gsClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(gsClientLogger);
		gsClientLogger->set_level(spdlog::level::trace);
		gsClientLogger->flush_on(spdlog::level::trace);
	}

}

