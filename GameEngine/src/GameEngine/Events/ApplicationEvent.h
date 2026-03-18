#pragma once

#include "GameEngine/Events/Event.h"

namespace GameEngine {

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: myWidth(width), myHeight(height) {}

		unsigned int GetWidth() const { return myWidth; }
		unsigned int GetHeight() const { return myHeight; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << myWidth << ", " << myHeight;
			return ss.str();
		}

		DECLARE_EVENT_TYPE(WindowResize)
		DECLARE_EVENT_CATEGORY(EventCategoryApplication)
	private:
		unsigned int myWidth, myHeight;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		DECLARE_EVENT_TYPE(WindowClose)
		DECLARE_EVENT_CATEGORY(EventCategoryApplication)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		DECLARE_EVENT_TYPE(AppTick)
		DECLARE_EVENT_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		DECLARE_EVENT_TYPE(AppUpdate)
		DECLARE_EVENT_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		DECLARE_EVENT_TYPE(AppRender)
		DECLARE_EVENT_CATEGORY(EventCategoryApplication)
	};
}