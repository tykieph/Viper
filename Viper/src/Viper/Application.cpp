#include "vpch.h"
#include "Application.h"

namespace Viper
{

	#define BIND_EVENT_FUNCTION(x) std::bind(&x, this, std::placeholders::_1)

	Application *Application::instance = nullptr;

	Application::Application()
	{
		V_CORE_ASSERT(!instance, "Application already exists!");
		instance = this;

		this->window = std::unique_ptr<Window>(Window::create());
		this->window->setEventCallback(BIND_EVENT_FUNCTION(Application::onEvent));
	}


	Application::~Application()
	{
	}

	void Application::run()
	{
		while (this->running)
		{
			for (Layer *layer : this->layers)
				layer->onUpdate();

			window->onUpdate();
		}
	}

	void Application::onEvent(Event &e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FUNCTION(Application::onWindowClose));
		
		for (auto it = this->layers.end(); it != this->layers.begin(); )
		{
			(*--it)->onEvent(e);
			if (e.handled)
				break;
		}

	}

	void Application::pushLayer(Layer *layer)
	{
		this->layers.pushLayer(layer);
	}

	void Application::pushOverlay(Layer *overlay)
	{
		this->layers.pushOverlay(overlay);
	}

	bool Application::onWindowClose(WindowCloseEvent &e)
	{
		this->running = false;
		return true;
	}

}