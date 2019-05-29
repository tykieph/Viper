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

#ifdef V_ENABLE_ASSERTS
	#define V_ASSERT(x, ...) {
		if( !(x) ) 
		{ 
			V_ERROR("Assertion failed: {0}", __VA_ARGS__); 
			__debugbreak(); 
		} }
	#define V_CORE_ASSERT(x, ...) { 
		if( !(x) ) 
		{ 
			V_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__);
			__debugbreak(); 
		} }
#else
	#define V_ASSERT(x, ...)
	#define V_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)