#pragma once

#include "Engine/Core/Base.h"
#include "Engine/Core/UUID.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/std.h>
#pragma warning(pop)

namespace GameEngine {

	class Log
	{
	public:
		static void Init();

		static Handle<spdlog::logger>& GetCoreLogger() { return gsCoreLogger; }
		static Handle<spdlog::logger>& GetClientLogger() { return gsClientLogger; }
	private:
		static Handle<spdlog::logger> gsCoreLogger;
		static Handle<spdlog::logger> gsClientLogger;
	};

}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

// fmt v10+ requires explicit formatter specializations; operator<< alone is not enough
template<glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> : fmt::ostream_formatter {};

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct fmt::formatter<glm::mat<C, R, T, Q>> : fmt::ostream_formatter {};

template<typename T, glm::qualifier Q>
struct fmt::formatter<glm::qua<T, Q>> : fmt::ostream_formatter {};

template<>
struct fmt::formatter<GameEngine::UUID> : fmt::formatter<uint64_t>
{
	auto format(const GameEngine::UUID& uuid, fmt::format_context& ctx) const
	{
		return fmt::formatter<uint64_t>::format((uint64_t)uuid, ctx);
	}
};

// Core log macros
#define GE_CORE_TRACE(...)    ::GameEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...)     ::GameEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...)     ::GameEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...)    ::GameEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_CRITICAL(...) ::GameEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define GE_TRACE(...)         ::GameEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GE_INFO(...)          ::GameEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define GE_WARN(...)          ::GameEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GE_ERROR(...)         ::GameEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define GE_CRITICAL(...)      ::GameEngine::Log::GetClientLogger()->critical(__VA_ARGS__)
