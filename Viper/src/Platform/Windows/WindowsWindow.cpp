#include "vpch.h"
#include "WindowsWindow.h"

namespace Viper
{

	static bool GLFWInitialized = false;

	Window *Window::create(const WindowProperties &properties)
	{
		return new WindowsWindow(properties);
	}

	WindowsWindow::WindowsWindow(const WindowProperties &properties)
	{
		this->init(properties);
	}

	WindowsWindow::~WindowsWindow()
	{
		this->shutdown();
	}

	void WindowsWindow::onUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(this->window);
	}

	void WindowsWindow::init(const WindowProperties &properties)
	{
		this->data.title = properties.title;
		this->data.width = properties.width;
		this->data.height = properties.height;

		V_CORE_INFO("Creating window {0} ({1}x{2})", this->data.title, this->data.width, this->data.height);

		if (!GLFWInitialized)
		{
			int success = glfwInit();
			V_CORE_ASSERT(success, "Could not initialize GLFW!");

			GLFWInitialized = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		this->window = glfwCreateWindow(this->data.width, this->data.height, this->data.title.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(this->window, this);
		//glfwSetFramebufferSizeCallback(this->window, this->framebufferResizeCallback);
	}

	void WindowsWindow::shutdown()
	{
		glfwDestroyWindow(this->window);
	}



}