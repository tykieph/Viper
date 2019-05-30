#pragma once

#include "Event.h"

namespace Viper
{

	class VIPER_API KeyEvent : public Event
	{
	public:
		inline uint16_t getKeyCode() const { return this->keyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard);

	protected:
		KeyEvent(uint16_t keyCode) : keyCode(keyCode) { }

	protected:
		uint16_t keyCode;
	};


	//*************** KeyPressedEvent class ***************//
	class VIPER_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(uint16_t keyCode, uint16_t repeatCount) : KeyEvent(keyCode), repeatCount(repeatCount) { }

		inline uint16_t getRepeatCount() const { return this->repeatCount; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << this->keyCode << " (" << this->repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed);

	private:
		uint16_t repeatCount;
	};


	//*************** KeyReleasedEvent class ***************//
	class VIPER_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(uint16_t keyCode) : KeyEvent(keyCode) { }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << this->keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased);
	};
}