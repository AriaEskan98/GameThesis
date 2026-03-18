include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "GameEngine"
	architecture "x86_64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/premake"
	include "GameEngine/vendor/Box2D"
	include "GameEngine/vendor/GLFW"
	include "GameEngine/vendor/Glad"
	include "GameEngine/vendor/msdf-atlas-gen"
	include "GameEngine/vendor/imgui-patch"
	include "GameEngine/vendor/yaml-cpp"
group ""

-- Vulkan SDK 1.4.x ships only /MD-compiled libs; override all vendor
-- projects to use runtime Release in Debug config to prevent LNK2038.
for _, name in ipairs({ "Box2D", "GLFW", "Glad", "msdf-atlas-gen",
                        "msdfgen", "freetype", "ImGui", "yaml-cpp" }) do
	project(name)
		filter "configurations:Debug"
			runtime "Release"
		filter {}
end

group "Core"
	include "GameEngine"
	include "GameEngine-ScriptCore"
group ""

group "Misc"
	include "Sandbox"
group ""
