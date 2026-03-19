
-- GameEngine Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["stb_image"]     = "%{wks.location}/GameEngine/vendor/stb_image"
IncludeDir["yaml_cpp"]      = "%{wks.location}/GameEngine/vendor/yaml-cpp/include"
IncludeDir["GLFW"]          = "%{wks.location}/GameEngine/vendor/GLFW/include"
IncludeDir["Glad"]          = "%{wks.location}/GameEngine/vendor/Glad/include"
IncludeDir["ImGui"]         = "%{wks.location}/GameEngine/vendor/ImGui"
IncludeDir["ImGuizmo"]      = "%{wks.location}/GameEngine/vendor/ImGuizmo"
IncludeDir["glm"]           = "%{wks.location}/GameEngine/vendor/glm"
IncludeDir["entt"]          = "%{wks.location}/GameEngine/vendor/entt/include"
IncludeDir["shaderc"]       = "%{wks.location}/GameEngine/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"]   = "%{wks.location}/GameEngine/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"]     = "%{VULKAN_SDK}/Include"
IncludeDir["assimp"]        = "%{wks.location}/GameEngine/vendor/assimp/include"
IncludeDir["assimp_config"] = "%{wks.location}/GameEngine/vendor/assimp/build/include"
IncludeDir["PhysX"]         = "%{wks.location}/GameEngine/vendor/PhysX/include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["assimp"]    = "%{wks.location}/GameEngine/vendor/assimp/build/lib"
LibraryDir["PhysX"]     = "%{wks.location}/GameEngine/vendor/PhysX/lib/%{cfg.buildcfg}"

Library = {}
Library["assimp"] = "%{LibraryDir.assimp}/assimp.lib"

Library["PhysX"]            = "%{LibraryDir.PhysX}/PhysX_static_64.lib"
Library["PhysXCommon"]      = "%{LibraryDir.PhysX}/PhysXCommon_static_64.lib"
Library["PhysXFoundation"]  = "%{LibraryDir.PhysX}/PhysXFoundation_static_64.lib"
Library["PhysXExtensions"]  = "%{LibraryDir.PhysX}/PhysXExtensions_static_64.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_combined.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_combined.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"
