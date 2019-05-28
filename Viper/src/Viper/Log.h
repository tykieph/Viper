#pragma once

#include "Core.h"

#include "spdlog/spdlog.h"

namespace Viper
{

	class VIPER_API Log
	{
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger> &getCoreLogger() { return coreLogger;  }
		inline static std::shared_ptr<spdlog::logger> &getClientLogger() { return clientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> coreLogger;
		static std::shared_ptr<spdlog::logger> clientLogger;
	};

}



//*************** Core log macros ***************//
#define V_CORE_INFO(...)		::Viper::Log::getCoreLogger()->info(__VA_ARGS__)
#define V_CORE_WARN(...)		::Viper::Log::getCoreLogger()->warn(__VA_ARGS__)
#define V_CORE_ERROR(...)		::Viper::Log::getCoreLogger()->error(__VA_ARGS__)
#define V_CORE_TRACE(...)		::Viper::Log::getCoreLogger()->trace(__VA_ARGS__)
#define V_CORE_FATAL(...)		::Viper::Log::getCoreLogger()->fatal(__VA_ARGS__)


//*************** Client log macros ***************//
#define V_INFO(...)			::Viper::Log::getClientLogger()->info(__VA_ARGS__)
#define V_WARN(...)			::Viper::Log::getClientLogger()->warn(__VA_ARGS__)
#define V_ERROR(...)		::Viper::Log::getClientLogger()->error(__VA_ARGS__)
#define V_TRACE(...)		::Viper::Log::getClientLogger()->trace(__VA_ARGS__)
#define V_FATAL(...)		::Viper::Log::getClientLogger()->fatal(__VA_ARGS__)
