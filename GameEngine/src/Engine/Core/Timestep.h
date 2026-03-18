#pragma once

namespace GameEngine {

	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			: myTime(time)
		{
		}

		operator float() const { return myTime; }

		float GetSeconds() const { return myTime; }
		float GetMilliseconds() const { return myTime * 1000.0f; }
	private:
		float myTime;
	};

}