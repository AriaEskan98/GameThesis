#include "gepch.h"
#include "Platform/Windows/WindowsWindow.h"

#include "GameEngine/Core/Input.h"

#include "GameEngine/Events/ApplicationEvent.h"
#include "GameEngine/Events/MouseEvent.h"
#include "GameEngine/Events/KeyEvent.h"

#include "GameEngine/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace GameEngine {
	
	static uint8_t gsGLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		GE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		GE_PROFILE_FUNCTION();

		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		GE_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		GE_PROFILE_FUNCTION();

		myData.Title = props.Title;
		myData.Width = props.Width;
		myData.Height = props.Height;

		GE_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (gsGLFWWindowCount == 0)
		{
			GE_PROFILE_SCOPE("glfwInit");
			int success = glfwInit();
			GE_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			GE_PROFILE_SCOPE("glfwCreateWindow");
		#if defined(GE_DEBUG)
			if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		#endif
			myWindow = glfwCreateWindow((int)props.Width, (int)props.Height, myData.Title.c_str(), nullptr, nullptr);
			++gsGLFWWindowCount;
		}

		myContext = GraphicsContext::Create(myWindow);
		myContext->Init();

		glfwSetWindowUserPointer(myWindow, &myData);
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(myWindow, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(myWindow, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(myWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(myWindow, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(myWindow, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(myWindow, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(myWindow, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void WindowsWindow::Shutdown()
	{
		GE_PROFILE_FUNCTION();

		glfwDestroyWindow(myWindow);
		--gsGLFWWindowCount;

		if (gsGLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}

	void WindowsWindow::OnUpdate()
	{
		GE_PROFILE_FUNCTION();

		glfwPollEvents();
		myContext->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		GE_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		myData.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return myData.VSync;
	}

	void WindowsWindow::SetCursorMode(CursorMode mode)
	{
		switch (mode)
		{
			case CursorMode::Normal: glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);   break;
			case CursorMode::Hidden: glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);   break;
			case CursorMode::Locked: glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); break;
		}
	}

}
