#pragma once

#include "vpch.h"

#include "Viper/Core.h"

namespace Viper
{

	struct WindowProperties
	{
		std::string title;
		unsigned int width;
		unsigned int height;

		WindowProperties(const std::string &title = "Viper Engine", unsigned int width = 1280, unsigned int height = 720) 
			: title(title), width(width), height(height) { }
	};

	class VIPER_API Window
	{
	public:
		virtual ~Window() {}

		virtual void onUpdate() = 0;

		virtual unsigned int getWidth() const = 0;
		virtual unsigned int getHeight() const = 0;

		// this function has to be created per platform
		static Window *create(const WindowProperties &properties = WindowProperties());
	};

}
