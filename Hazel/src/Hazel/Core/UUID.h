#pragma once

namespace GameEngine {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return myUUID; }
	private:
		uint64_t myUUID;
	};

}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<GameEngine::UUID>
	{
		std::size_t operator()(const GameEngine::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}
