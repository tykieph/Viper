#pragma once

#include "Event.h"

namespace Viper
{

	//*************** WindowResizeEvent class ***************//
	class VIPER_API WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint16_t width, uint16_t height) : width(width), height(height) { }

		inline uint16_t getWidth() const { return this->width; };
		inline uint16_t getHeight() const { return this->height; };

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << this->width << ", " << this->height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	private:
		uint16_t width, height;
	};


	//*************** WindowCloseEvent class ***************//
	class VIPER_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() { }

		EVENT_CLASS_TYPE(WindowClose);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};


	//*************** AppTickEvent class ***************//
	class VIPER_API AppTickEvent : public Event
	{
	public:
		AppTickEvent() { }

		EVENT_CLASS_TYPE(AppTick);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};


	//*************** AppUpdateEvent class ***************//
	class VIPER_API AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() { }

		EVENT_CLASS_TYPE(AppUpdate);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};


	//*************** AppRenderEvent class ***************//
	class VIPER_API AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() { }

		EVENT_CLASS_TYPE(AppRender);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};
}