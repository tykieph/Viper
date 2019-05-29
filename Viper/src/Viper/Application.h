#pragma once

#include "Core.h"
#include "Window.h"

namespace Viper
{

	class VIPER_API Application
	{
	public:
		Application();
		virtual ~Application();

		void run();

	private:
		std::unique_ptr<Window> window;
		bool running = true;
	};

	// to be defined in CLIENT
	Application *createApplication();

}