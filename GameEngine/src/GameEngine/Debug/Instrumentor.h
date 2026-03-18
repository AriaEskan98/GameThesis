#pragma once

#include "GameEngine/Core/Log.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

namespace GameEngine {

	using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult
	{
		std::string Name;

		FloatingPointMicroseconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	class Profiler
	{
	public:
		Profiler(const Profiler&) = delete;
		Profiler(Profiler&&) = delete;

		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			std::lock_guard lock(myMutex);
			if (myCurrentSession)
			{
				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the
				// newly opened session instead.  That's better than having badly formatted
				// profiling output.
				if (Log::GetCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
				{
					GE_CORE_ERROR("Profiler::BeginSession('{0}') when session '{1}' already open.", name, myCurrentSession->Name);
				}
				InternalEndSession();
			}
			myOutputStream.open(filepath);

			if (myOutputStream.is_open())
			{
				myCurrentSession = new InstrumentationSession({name});
				WriteHeader();
			}
			else
			{
				if (Log::GetCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
				{
					GE_CORE_ERROR("Profiler could not open results file '{0}'.", filepath);
				}
			}
		}

		void EndSession()
		{
			std::lock_guard lock(myMutex);
			InternalEndSession();
		}

		void WriteProfile(const ProfileResult& result)
		{
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
			json << "\"name\":\"" << result.Name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.ThreadID << ",";
			json << "\"ts\":" << result.Start.count();
			json << "}";

			std::lock_guard lock(myMutex);
			if (myCurrentSession)
			{
				myOutputStream << json.str();
				myOutputStream.flush();
			}
		}

		static Profiler& GetInstance()
		{
			static Profiler instance;
			return instance;
		}
	private:
		Profiler()
			: myCurrentSession(nullptr)
		{
		}

		~Profiler()
		{
			EndSession();
		}		

		void WriteHeader()
		{
			myOutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			myOutputStream.flush();
		}

		void WriteFooter()
		{
			myOutputStream << "]}";
			myOutputStream.flush();
		}

		// Note: you must already own lock on myMutex before
		// calling InternalEndSession()
		void InternalEndSession()
		{
			if (myCurrentSession)
			{
				WriteFooter();
				myOutputStream.close();
				delete myCurrentSession;
				myCurrentSession = nullptr;
			}
		}
	private:
		std::mutex myMutex;
		InstrumentationSession* myCurrentSession;
		std::ofstream myOutputStream;
	};

	class ProfileTimer
	{
	public:
		ProfileTimer(const char* name)
			: myName(name), myStopped(false)
		{
			myStartTimepoint = std::chrono::steady_clock::now();
		}

		~ProfileTimer()
		{
			if (!myStopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::steady_clock::now();
			auto highResStart = FloatingPointMicroseconds{ myStartTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(myStartTimepoint).time_since_epoch();

			Profiler::GetInstance().WriteProfile({ myName, highResStart, elapsedTime, std::this_thread::get_id() });

			myStopped = true;
		}
	private:
		const char* myName;
		std::chrono::time_point<std::chrono::steady_clock> myStartTimepoint;
		bool myStopped;
	};

	namespace ProfilerUtils {

		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;
				if (matchIndex == K - 1)
					srcIndex += matchIndex;
				result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}
			return result;
		}
	}
}

#define GE_PROFILE 0
#if GE_PROFILE
	// Resolve which function signature macro will be used. Note that this only
	// is resolved when the (pre)compiler starts, so the syntax highlighting
	// could mark the wrong one in your editor!
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define GE_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define GE_FUNC_SIG __PRETTY_FUNCTION__
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define GE_FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define GE_FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define GE_FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define GE_FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define GE_FUNC_SIG __func__
	#else
		#define GE_FUNC_SIG "GE_FUNC_SIG unknown!"
	#endif

	#define GE_PROFILE_BEGIN_SESSION(name, filepath) ::GameEngine::Profiler::GetInstance().BeginSession(name, filepath)
	#define GE_PROFILE_END_SESSION() ::GameEngine::Profiler::GetInstance().EndSession()
	#define GE_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::GameEngine::ProfilerUtils::CleanupOutputString(name, "__cdecl ");\
											   ::GameEngine::ProfileTimer timer##line(fixedName##line.Data)
	#define GE_PROFILE_SCOPE_LINE(name, line) GE_PROFILE_SCOPE_LINE2(name, line)
	#define GE_PROFILE_SCOPE(name) GE_PROFILE_SCOPE_LINE(name, __LINE__)
	#define GE_PROFILE_FUNCTION() GE_PROFILE_SCOPE(GE_FUNC_SIG)
#else
	#define GE_PROFILE_BEGIN_SESSION(name, filepath)
	#define GE_PROFILE_END_SESSION()
	#define GE_PROFILE_SCOPE(name)
	#define GE_PROFILE_FUNCTION()
#endif