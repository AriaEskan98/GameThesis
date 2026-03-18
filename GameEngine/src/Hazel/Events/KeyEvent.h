#pragma once

#include "Hazel/Events/Event.h"
#include "Hazel/Core/KeyCodes.h"

namespace GameEngine {

	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return myKeyCode; }

		GE_EVENT_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(const KeyCode keycode)
			: myKeyCode(keycode) {}

		KeyCode myKeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
			: KeyEvent(keycode), myIsRepeat(isRepeat) {}

		bool IsRepeat() const { return myIsRepeat; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << myKeyCode << " (repeat = " << myIsRepeat << ")";
			return ss.str();
		}

		GE_EVENT_TYPE(KeyPressed)
	private:
		bool myIsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << myKeyCode;
			return ss.str();
		}

		GE_EVENT_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << myKeyCode;
			return ss.str();
		}

		GE_EVENT_TYPE(KeyTyped)
	};
}
