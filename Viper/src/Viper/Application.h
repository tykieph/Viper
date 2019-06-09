#pragma once

#include "Core.h"
#include "Window.h"
#include "Input.h"

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

		virtual void onEvent(Event &e);

		inline Window &getWindow() const { return *this->window; }
		inline static Application &get() { return *instance; }

	private:
		bool onWindowClose(WindowCloseEvent &e);

	private:
		std::unique_ptr<Window> window;
		bool running = true;

		static Application *instance;
	};

	// to be defined in CLIENT
	Application *createApplication();

}