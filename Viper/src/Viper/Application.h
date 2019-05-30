#pragma once

#include "Core.h"
#include "Window.h"

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

namespace Viper
{

	class VIPER_API Application
	{
	public:
		Application();
		virtual ~Application();

		void run();

		void onEvent(Event &e);

	private:
		bool onWindowClose(WindowCloseEvent &e);

	private:
		std::unique_ptr<Window> window;
		bool running = true;
	};

	// to be defined in CLIENT
	Application *createApplication();

}