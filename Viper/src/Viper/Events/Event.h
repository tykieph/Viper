#pragma once

#include "vpch.h"
#include "Viper/Core.h"

namespace Viper
{

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication		= BIT(0),
		EventCategoryInput				= BIT(1),
		EventCategoryKeyboard			= BIT(2),
		EventCategoryMouse				= BIT(3),
		EventCategoryMouseButton		= BIT(4)
	};

	#define EVENT_CLASS_TYPE(type)  static EventType getStaticType() { return EventType::##type; }\
									virtual EventType getEventType() const override { return getStaticType(); }\
									virtual const char *getName() const override { return #type; }

	#define EVENT_CLASS_CATEGORY(category) virtual uint32_t getCategoryFlags() const override { return category; }

	//*************** Event class ***************//
	class VIPER_API Event
	{
		friend class EventDispatcher;

	public:
		virtual EventType getEventType() const = 0;
		virtual const char *getName() const = 0;
		virtual uint32_t getCategoryFlags() const = 0;
		virtual std::string toString() const { return this->getName(); }

		inline bool isInCategory(EventCategory category)
		{
			return getCategoryFlags() & category;
		}

	public:
		bool handled = false;	
	};


	//*************** EventDispatcher class ***************//
	class EventDispatcher
	{
		template<typename T>
		using eventFunc = std::function<bool(T&)>;

	public:
		EventDispatcher(Event &e) : e(e) { }

		template<typename T>
		bool dispatch(eventFunc<T> func)
		{
			if (e.getEventType() == T::getStaticType())
			{
				e.handled = func(*(T*)&e);
				return true;
			}
			return false;
		}

	private:
		Event &e;
	};

	inline std::ostream &operator<<(std::ostream &os, const Event &e)
	{
		return os << e.toString();
	}
}