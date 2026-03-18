#pragma once

#include "Engine/Core/Base.h"
#include "Engine/Events/Event.h"

#include <sstream>

namespace GameEngine {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "GameEngine",
			        uint32_t width = 1600,
			        uint32_t height = 900)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	/// Controls how the OS cursor behaves inside the window.
	enum class CursorMode
	{
		Normal  = 0, ///< Visible, free to move.
		Hidden  = 1, ///< Hidden but not locked (position still bounded).
		Locked  = 2  ///< Hidden and locked; position is virtual (FPS mode).
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		/// Lock / hide the OS cursor for FPS-style input capture.
		virtual void SetCursorMode(CursorMode mode) = 0;

		static Own<Window> Create(const WindowProps& props = WindowProps());
	};

}
