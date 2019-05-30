#pragma once

#include "Event.h"

namespace Viper
{

	//*************** MouseMovedEvent class ***************//
	class VIPER_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float_t x, float_t y) : xpos(x), ypos(y) { }

		inline float_t getX() const { return this->xpos; }
		inline float_t getY() const { return this->ypos; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << this->xpos << ", " << this->ypos;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved);
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse);

	private:
		float_t xpos, ypos;
	};


	//*************** MouseScrolledEvent class ***************//
	class VIPER_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float_t xOffset, float_t yOffset) : xoffset(xOffset), yoffset(yOffset) { }

		inline float_t getXOffset() const { return this->xoffset; }
		inline float_t getYOffset() const { return this->yoffset; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << this->xoffset << ", " << this->yoffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled);
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse);

	private:
		float_t xoffset, yoffset;
	};


	//*************** MouseButtonEvent class ***************//
	class VIPER_API MouseButtonEvent : public Event
	{
	public:
		inline uint16_t getMouseButton() const { return this->button; }

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse);

	protected:
		MouseButtonEvent(uint16_t button) : button(button) { }

	protected:
		uint16_t button;
	};


	//*************** MouseButtonPressedEvent class ***************//
	class VIPER_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(uint16_t button) : MouseButtonEvent(button) { }
		
		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << this->button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed);
	};


	//*************** MouseButtonReleasedEvent class ***************//
	class VIPER_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(uint16_t button) : MouseButtonEvent(button) { }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << this->button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased);
	};
}