#include "vpch.h"
#include "Application.h"

namespace Viper
{

	Application::Application()
	{
		this->window = std::unique_ptr<Window>(Window::create());
	}


	Application::~Application()
	{
	}

	void Application::run()
	{

		while (this->running)
		{
			window->onUpdate();
		}
	}

}