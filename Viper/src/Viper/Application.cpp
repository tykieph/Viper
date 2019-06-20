#include "vpch.h"
#include "Application.h"

// testing purposes
#include "Viper/Renderer/GraphicsContext.h"

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
		this->window->~Window();
	}

	void Application::run()
	{
		while (this->running)
		{
			// testing
			if (Viper::Input::isKeyPressed(V_KEY_TAB))
			{
				auto context = static_cast<GraphicsContext *>(this->window->getContextHandle());
				float mousex = (2.0f * Input::getMouseX() + 1.0f) / this->window->getWidth() - 1.0f;
				float mousey = (2.0f * Input::getMouseY() + 1.0f) / this->window->getHeight() - 1.0f;

				context->updateVertices(std::pair<float, float>(mousex, mousey - 0.1f),
										std::pair<float, float>(mousex + 0.1f, mousey + 0.1f),
										std::pair<float, float>(mousex - 0.1f, mousey + 0.1f));
			}
			
			for (Layer *layer : this->layers)
				layer->onUpdate();

			window->onUpdate();
		}
	}

	void Application::onEvent(Event &e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FUNCTION(Application::onWindowClose));

		// testing
		dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNCTION(Application::onMouseButtonPressed));

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

	// testing
	bool Application::onMouseButtonPressed(MouseButtonPressedEvent &e)
	{
		V_CORE_INFO("Mouse button pressed! {0}, {1};{2}", e, Input::getMouseX(), Input::getMouseY());
		return true;
	}

}