project "ImGui"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		-- All files from the imgui submodule except imgui.cpp (which has a bug)
		"../imgui/imconfig.h",
		"../imgui/imgui.h",
		"../imgui/imgui_draw.cpp",
		"../imgui/imgui_internal.h",
		"../imgui/imgui_tables.cpp",
		"../imgui/imgui_widgets.cpp",
		"../imgui/imstb_rectpack.h",
		"../imgui/imstb_textedit.h",
		"../imgui/imstb_truetype.h",
		"../imgui/imgui_demo.cpp",

		-- Patched imgui.cpp (fixes 'DC' undeclared identifier in SetActiveID)
		"imgui.cpp"
	}

	includedirs
	{
		"../imgui"
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
