project "zlib"
	kind "StaticLib"
	language "C"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir    ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"adler32.c",
		"compress.c",
		"crc32.c",
		"deflate.c",
		"gzclose.c",
		"gzlib.c",
		"gzread.c",
		"gzwrite.c",
		"infback.c",
		"inffast.c",
		"inflate.c",
		"inftrees.c",
		"trees.c",
		"uncompr.c",
		"zutil.c",
	}

	defines { "_CRT_SECURE_NO_WARNINGS" }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Release"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
