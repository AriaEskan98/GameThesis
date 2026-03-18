#pragma once

#include "GameEngine/Debug/Profiler.h"
#include "GameEngine/Core/Base.h"

#include <functional>

namespace GameEngine {

	// Events in GameEngine are currently blocking, meaning when an event occurs it
	// immediately gets dispatched and must be dealt with right then an there.
	// For the future, a better strategy might be to buffer events in an event
	// bus and process them during the "event" part of the update stage.

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory : int
	{
		None                        = 0,
		EventCategoryApplication    = (1 << 0),
		EventCategoryInput          = (1 << 1),
		EventCategoryKeyboard       = (1 << 2),
		EventCategoryMouse          = (1 << 3),
		EventCategoryMouseButton    = (1 << 4)
	};

#define GE_EVENT_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define GE_EVENT_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category) const
		{
			return (GetCategoryFlags() & category) != 0;
		}
	};

	class EventDispatcher
	{
	public:
		explicit EventDispatcher(Event& event)
			: myEvent(event)
		{
		}

		template<typename T>
		bool Dispatch(const std::function<bool(T&)>& func)
		{
			if (myEvent.GetEventType() != T::GetStaticType())
				return false;

			myEvent.Handled |= func(static_cast<T&>(myEvent));
			return true;
		}
	private:
		Event& myEvent;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

}

