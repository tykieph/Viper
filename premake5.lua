workspace "Viper"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder
IncludeDir = {}
IncludeDir["GLFW"] = "Viper/vendor/GLFW/include"

include "Viper/vendor/GLFW"

project "Viper"
	location "Viper"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"	

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intdir/" .. outputdir .. "/%{prj.name}")

	pchheader "vpch.h"
	pchsource "%{prj.name}/src/vpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/glm",
		"%{IncludeDir.GLFW}",
		"C:/VulkanSDK/1.1.106.0/Include"
	}

	links
	{
		"GLFW",
		"vulkan-1"
	}

	libdirs
	{
		"C:/VulkanSDK/1.1.106.0/Lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"V_PLATFORM_WINDOWS",
			"V_BUILD_DLL",
			"GLFW_INCLUDE_VULKAN"
		}

	filter "configurations:Debug"
		defines "V_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "V_RELEASE"
		optimize "on"

	filter "configurations:Dist"
		defines "V_DIST"
		optimize "on"



project "Game"
	location "Game"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intdir/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Viper/src",
		"Viper/vendor/spdlog/include"
	}

	links
	{
		"Viper"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"V_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "V_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "V_RELEASE"
		optimize "on"

	filter "configurations:Dist"
		defines "V_DIST"
		optimize "on"
