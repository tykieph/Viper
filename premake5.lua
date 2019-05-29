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
	kind "SharedLib"
	language "C++"

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
		"%{IncludeDir.GLFW}",
		"C:/VulkanSDK/1.1.106.0/Include"
	}

	links
	{
		"GLFW"
	}

	libdirs
	{
		"C:/VulkanSDK/1.1.106.0/Lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"V_PLATFORM_WINDOWS",
			"V_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Game")
		} 

	filter "configurations:Debug"
		defines "V_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "V_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "V_DIST"
		optimize "On"



project "Game"
	location "Game"
	kind "ConsoleApp"
	language "C++"

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
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"V_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "V_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "V_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "V_DIST"
		optimize "On"
