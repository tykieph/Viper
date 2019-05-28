#pragma once

#ifdef V_PLATFORM_WINDOWS
	#ifdef V_BUILD_DLL
		#define VIPER_API __declspec(dllexport)
	#else 
		#define VIPER_API __declspec(dllimport)
	#endif
#else
	#error Viper supports only Windows
#endif

