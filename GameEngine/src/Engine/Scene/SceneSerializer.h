#pragma once

#include "Scene.h"

namespace GameEngine {

	class SceneSerializer
	{
	public:
		SceneSerializer(const Handle<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
	private:
		Handle<Scene> myScene;
	};

}
