#pragma once

#include "Viper/Window.h"

#include <GLFW/glfw3.h>

namespace Viper
{

	class VIPER_API WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties &properties);
		virtual ~WindowsWindow();

		void onUpdate() override;

		inline unsigned int getWidth() const override { return data.width; }
		inline unsigned int getHeight() const override { return data.height; }

	private:
		virtual void init(const WindowProperties &properties);
		virtual void shutdown();

	private:
		GLFWwindow *window;

		struct WindowData
		{
			std::string title;
			unsigned int width;
			unsigned int height;
		};

		WindowData data;
	};

}