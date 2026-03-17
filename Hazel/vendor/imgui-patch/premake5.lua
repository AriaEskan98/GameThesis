local imgui_dir = _SCRIPT_DIR .. "/../imgui/"

project "ImGui"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		-- All files from the imgui submodule except imgui.cpp (which has a bug)
		imgui_dir .. "imconfig.h",
		imgui_dir .. "imgui.h",
		imgui_dir .. "imgui_draw.cpp",
		imgui_dir .. "imgui_internal.h",
		imgui_dir .. "imgui_tables.cpp",
		imgui_dir .. "imgui_widgets.cpp",
		imgui_dir .. "imstb_rectpack.h",
		imgui_dir .. "imstb_textedit.h",
		imgui_dir .. "imstb_truetype.h",
		imgui_dir .. "imgui_demo.cpp",

		-- Patched imgui.cpp (fixes 'DC' undeclared identifier in SetActiveID)
		_SCRIPT_DIR .. "/imgui.cpp"
	}

	includedirs
	{
		imgui_dir
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		buildoptions { "/utf-8" }

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
		symbols "off"
