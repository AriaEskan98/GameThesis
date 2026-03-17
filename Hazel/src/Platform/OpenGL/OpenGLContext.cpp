#include "gepch.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace GameEngine {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: myWindowHandle(windowHandle)
	{
		GE_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		GE_PROFILE_FUNCTION();

		glfwMakeContextCurrent(myWindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		GE_CORE_ASSERT(status, "Failed to initialize Glad!");

		GE_CORE_INFO("OpenGL Info:");
		GE_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		GE_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		GE_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

		GE_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "GameEngine requires at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		GE_PROFILE_FUNCTION();

		glfwSwapBuffers(myWindowHandle);
	}

}
