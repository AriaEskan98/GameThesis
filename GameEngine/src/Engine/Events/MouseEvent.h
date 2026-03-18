#pragma once

#include "Engine/Events/Event.h"
#include "Engine/Core/MouseCodes.h"

namespace GameEngine {

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float x, const float y)
			: myMouseX(x), myMouseY(y) {}

		float GetX() const { return myMouseX; }
		float GetY() const { return myMouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << myMouseX << ", " << myMouseY;
			return ss.str();
		}

		GE_EVENT_TYPE(MouseMoved)
		GE_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float myMouseX, myMouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset)
			: myXOffset(xOffset), myYOffset(yOffset) {}

		float GetXOffset() const { return myXOffset; }
		float GetYOffset() const { return myYOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		GE_EVENT_TYPE(MouseScrolled)
		GE_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float myXOffset, myYOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return myButton; }

		GE_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
	protected:
		MouseButtonEvent(const MouseCode button)
			: myButton(button) {}

		MouseCode myButton;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << myButton;
			return ss.str();
		}

		GE_EVENT_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << myButton;
			return ss.str();
		}

		GE_EVENT_TYPE(MouseButtonReleased)
	};

}
