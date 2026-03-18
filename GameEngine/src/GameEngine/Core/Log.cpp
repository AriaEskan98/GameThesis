#include "gepch.h"
#include "GameEngine/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace GameEngine {

	Handle<spdlog::logger> Log::gsCoreLogger;
	Handle<spdlog::logger> Log::gsClientLogger;

	void Log::Init()
	{
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("%^[%T] %n: %v%$");

		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("GameEngine.log", true);
		fileSink->set_pattern("[%T] [%l] %n: %v");

		spdlog::sinks_init_list sinks{ consoleSink, fileSink };

		gsCoreLogger = std::make_shared<spdlog::logger>("ENGINE", sinks);
		spdlog::register_logger(gsCoreLogger);
		gsCoreLogger->set_level(spdlog::level::trace);
		gsCoreLogger->flush_on(spdlog::level::trace);

		gsClientLogger = std::make_shared<spdlog::logger>("APP", sinks);
		spdlog::register_logger(gsClientLogger);
		gsClientLogger->set_level(spdlog::level::trace);
		gsClientLogger->flush_on(spdlog::level::trace);
	}

}
