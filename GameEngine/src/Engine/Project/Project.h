#pragma once

#include <string>
#include <filesystem>

#include "Engine/Core/Base.h"

namespace GameEngine {

	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetProjectDirectory()
		{
			GE_CORE_ASSERT(gsActiveProject);
			return gsActiveProject->myProjectDirectory;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			GE_CORE_ASSERT(gsActiveProject);
			return GetProjectDirectory() / gsActiveProject->myConfig.AssetDirectory;
		}

		// TODO(Yan): move to asset manager when we have one
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			GE_CORE_ASSERT(gsActiveProject);
			return GetAssetDirectory() / path;
		}

		ProjectConfig& GetConfig() { return myConfig; }

		static Handle<Project> GetActive() { return gsActiveProject; }

		static Handle<Project> New();
		static Handle<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig myConfig;
		std::filesystem::path myProjectDirectory;

		inline static Handle<Project> gsActiveProject;
	};

}
