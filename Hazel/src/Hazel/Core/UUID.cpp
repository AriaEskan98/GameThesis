#include "gepch.h"
#include "UUID.h"

#include <random>

#include <unordered_map>

namespace GameEngine {

	static std::random_device gsRandomDevice;
	static std::mt19937_64 gsEngine(gsRandomDevice());
	static std::uniform_int_distribution<uint64_t> gsUniformDistribution;

	UUID::UUID()
		: myUUID(gsUniformDistribution(gsEngine))
	{
	}

	UUID::UUID(uint64_t uuid)
		: myUUID(uuid)
	{
	}

}
