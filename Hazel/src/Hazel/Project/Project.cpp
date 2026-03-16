#include "gepch.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace GameEngine {

	Handle<Project> Project::New()
	{
		gsActiveProject = MakeHandle<Project>();
		return gsActiveProject;
	}

	Handle<Project> Project::Load(const std::filesystem::path& path)
	{
		Handle<Project> project = MakeHandle<Project>();

		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->myProjectDirectory = path.parent_path();
			gsActiveProject = project;
			return gsActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(gsActiveProject);
		if (serializer.Serialize(path))
		{
			gsActiveProject->myProjectDirectory = path.parent_path();
			return true;
		}

		return false;
	}

}
