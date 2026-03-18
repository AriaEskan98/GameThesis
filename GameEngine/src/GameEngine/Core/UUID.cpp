#include "gepch.h"
#include "UUID.h"

#include <chrono>

namespace GameEngine {

	static uint64_t XorShift64(uint64_t& state)
	{
		state ^= state << 13;
		state ^= state >> 7;
		state ^= state << 17;
		return state;
	}

	static uint64_t gsUUIDState = []()
	{
		uint64_t seed = static_cast<uint64_t>(
			std::chrono::high_resolution_clock::now().time_since_epoch().count()
		);
		// Avalanche the seed bits to avoid poor initial states
		seed ^= seed >> 33;
		seed *= 0xff51afd7ed558ccdULL;
		seed ^= seed >> 33;
		seed *= 0xc4ceb9fe1a85ec53ULL;
		seed ^= seed >> 33;
		return seed ? seed : 0xdeadbeefcafe1337ULL;
	}();

	UUID::UUID()
		: myUUID(XorShift64(gsUUIDState))
	{
	}

	UUID::UUID(uint64_t uuid)
		: myUUID(uuid)
	{
	}

}
