#pragma once

#include "Viper/Window.h"
#include "Viper/Renderer/GraphicsContext.h"

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
		inline bool getFramebufferResizeState() const override { return data.framebufferResized; }

		inline void setEventCallback(const eventCallbackFunc &callback) override { this->data.eventCallback = callback; }
		inline void setFramebufferResizeState(bool framebufferResized) override { this->data.framebufferResized = framebufferResized; }

		inline void *getNativeWindow() const override { return this->window; }
		inline void *getContextHandle() const override { return this->context; }

	private:
		virtual void init(const WindowProperties &properties);
		virtual void shutdown();

	private:
		GLFWwindow *window;
		GraphicsContext *context;

		struct WindowData
		{
			std::string title;
			uint16_t width;
			uint16_t height;
			bool framebufferResized;

			eventCallbackFunc eventCallback;
		};

		WindowData data;
	};

}