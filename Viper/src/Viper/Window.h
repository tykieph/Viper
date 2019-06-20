#pragma once

#include "vpch.h"

#include "Viper/Core.h"
#include "Viper/Events/Event.h"

namespace Viper
{

	struct WindowProperties
	{
		std::string title;
		uint16_t width;
		uint16_t height;

		WindowProperties(const std::string &title = "Viper Engine", uint16_t width = 1280, uint16_t height = 720)
			: title(title), width(width), height(height) { }
	};

	class VIPER_API Window
	{
	public:
		using eventCallbackFunc = std::function<void(Event&)>;

		virtual ~Window() { }

		virtual void onUpdate() = 0;

		virtual uint16_t getWidth() const = 0;
		virtual uint16_t getHeight() const = 0;
		virtual bool getFramebufferResizeState() const = 0;

		virtual void setEventCallback(const eventCallbackFunc &callback) = 0;
		virtual void setFramebufferResizeState(bool framebufferResized) = 0;

		virtual void *getNativeWindow() const = 0;
		virtual void *getContextHandle() const = 0;

		// this function has to be created per platform
		static Window *create(const WindowProperties &properties = WindowProperties());
	};

}
