#include "vpch.h"
#include "WindowsWindow.h"

#include "Viper/Events/ApplicationEvent.h"
#include "Viper/Events/MouseEvent.h"
#include "Viper/Events/KeyEvent.h"

#include "Platform/Vulkan/VulkanContext.h"

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
		this->context->swapBuffers();
	}

	void WindowsWindow::init(const WindowProperties &properties)
	{
		this->data.title = properties.title;
		this->data.width = properties.width;
		this->data.height = properties.height;
		this->data.framebufferResized = false;

		V_CORE_INFO("Creating window {0} ({1}x{2})", this->data.title, this->data.width, this->data.height);

		if (!GLFWInitialized)
		{
			int success = glfwInit();
			V_CORE_ASSERT(success, "Could not initialize GLFW!");

			// Set GLFW error callback
			glfwSetErrorCallback([](int error, const char *description)
			{
				V_CORE_ERROR("GLFW error ({0}): {1}", error, description);
			});

			GLFWInitialized = true;
		}


		// initialize Vulkan
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		this->window = glfwCreateWindow(this->data.width, this->data.height, this->data.title.c_str(), nullptr, nullptr);

		this->context = new VulkanContext(this);
		this->context->init();

		
		// Set GLFW callbacks 
		glfwSetWindowUserPointer(this->window, &this->data);

		glfwSetFramebufferSizeCallback(this->window, [](GLFWwindow *window, int width, int height)
		{
			auto app = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		});

		glfwSetWindowSizeCallback(this->window, [](GLFWwindow *window, int width, int height)
		{
			WindowData *data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->width = width;
			data->height = height;

			WindowResizeEvent e(width, height);
			data->eventCallback(e);
		}); 

		glfwSetWindowCloseCallback(this->window, [](GLFWwindow *window)
		{
			WindowData *data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			WindowCloseEvent e;
			data->eventCallback(e);
		});

		glfwSetKeyCallback(this->window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			WindowData *data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent e(key, 0);
					data->eventCallback(e);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent e(key);
					data->eventCallback(e);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent e(key, 1);
					data->eventCallback(e);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(this->window, [](GLFWwindow *window, int button, int action, int mods)
		{
			WindowData *data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent e(button);
					data->eventCallback(e);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent e(button);
					data->eventCallback(e);
					break;
				}
			}
		});

		glfwSetScrollCallback(this->window, [](GLFWwindow *window, double_t xOffset, double_t yOffset)
		{
			WindowData *data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent e((float_t)xOffset, (float_t)yOffset);
			data->eventCallback(e);
		});

		glfwSetCursorPosCallback(this->window, [](GLFWwindow *window, double_t xPos, double_t yPos)
		{
			WindowData *data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseMovedEvent e((float_t)xPos, (float_t)yPos);
			data->eventCallback(e);
		});

	}

	void WindowsWindow::shutdown()
	{
		glfwDestroyWindow(this->window);
		delete this->context;
		glfwTerminate();
	}

}