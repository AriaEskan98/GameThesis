#pragma once

#include "Project.h"

namespace GameEngine {

	class ProjectSerializer
	{
	public:
		ProjectSerializer(Handle<Project> project);

		bool Serialize(const std::filesystem::path& filepath);
		bool Deserialize(const std::filesystem::path& filepath);
	private:
		Handle<Project> myProject;
	};

}
