#pragma once

#include "Engine/Events/Event.h"

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

		GE_EVENT_TYPE(WindowResize)
		GE_EVENT_CATEGORY(EventCategoryApplication)
	private:
		unsigned int myWidth, myHeight;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		GE_EVENT_TYPE(WindowClose)
		GE_EVENT_CATEGORY(EventCategoryApplication)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		GE_EVENT_TYPE(AppTick)
		GE_EVENT_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		GE_EVENT_TYPE(AppUpdate)
		GE_EVENT_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		GE_EVENT_TYPE(AppRender)
		GE_EVENT_CATEGORY(EventCategoryApplication)
	};
}