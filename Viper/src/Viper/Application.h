#pragma once

#include "Viper/Core.h"
#include "Viper/Window.h"
#include "Viper/Input.h"
#include "Viper/LayerStack.h"
#include "Viper/KeyCodes.h"

#include "Viper/Events/Event.h"
#include "Viper/Events/ApplicationEvent.h"

// testing
#include "Viper/Events/MouseEvent.h"

namespace Viper
{

	class VIPER_API Application
	{
	public:
		Application();
		virtual ~Application();

		void run();

		void onEvent(Event &e);

		void pushLayer(Layer *layer);
		void pushOverlay(Layer *overlay);

		inline Window &getWindow() const { return *this->window; }
		inline static Application &get() { return *instance; }

	private:
		bool onWindowClose(WindowCloseEvent &e);

		// testing
		bool onMouseButtonPressed(MouseButtonPressedEvent &e);

	private:
		std::unique_ptr<Window> window;
		bool running = true;
		LayerStack layers;

		static Application *instance;
	};

	// to be defined in CLIENT
	Application *createApplication();

}