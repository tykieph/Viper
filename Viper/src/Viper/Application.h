#pragma once

#include "Core.h"

namespace Viper
{

	class VIPER_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void run();
	};

	// to be defined in CLIENT
	Application *createApplication();

}