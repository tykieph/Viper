#pragma once

#include "Viper/Window.h"

#include "Viper/Events/ApplicationEvent.h"
#include "Viper/Events/MouseEvent.h"
#include "Viper/Events/KeyEvent.h"

#include <GLFW/glfw3.h>

namespace Viper
{

	class VIPER_API WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties &properties);
		virtual ~WindowsWindow();

		void onUpdate() override;

		inline uint16_t getWidth() const override { return data.width; }
		inline uint16_t getHeight() const override { return data.height; }

		inline void setEventCallback(const eventCallbackFunc &callback) override { this->data.eventCallback = callback; }

	private:
		virtual void init(const WindowProperties &properties);
		virtual void shutdown();

	private:
		GLFWwindow *window;

		struct WindowData
		{
			std::string title;
			uint16_t width;
			uint16_t height;

			eventCallbackFunc eventCallback;
		};

		WindowData data;
	};

}