include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Hazel"
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
	include "Hazel/vendor/Box2D"
	include "Hazel/vendor/GLFW"
	include "Hazel/vendor/Glad"
	include "Hazel/vendor/msdf-atlas-gen"
	include "Hazel/vendor/imgui-patch"
	include "Hazel/vendor/yaml-cpp"
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
	include "Hazel"
	include "Hazel-ScriptCore"
group ""

group "Misc"
	include "Sandbox"
group ""
