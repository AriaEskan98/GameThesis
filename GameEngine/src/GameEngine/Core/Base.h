#pragma once

#include "GameEngine/Core/PlatformDetection.h"

#include <memory>

#ifdef GE_DEBUG
	#if defined(GE_PLATFORM_WINDOWS)
		#define GE_DEBUGBREAK() __debugbreak()
	#elif defined(GE_PLATFORM_LINUX)
		#include <signal.h>
		#define GE_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define GE_ENABLE_ASSERTS
#else
	#define GE_DEBUGBREAK()
#endif

#define GE_EXPAND_MACRO(x) x
#define GE_STRINGIFY_MACRO(x) #x

#define FLAG(x) (1 << x)

#define GE_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace GameEngine {

	template<typename T>
	using Own = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Own<T> MakeOwn(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Handle = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Handle<T> MakeHandle(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#include "GameEngine/Core/Log.h"
#include "GameEngine/Core/Assert.h"
