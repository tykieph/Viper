#include "vpch.h"
#include "WindowsInput.h"

#include "Viper/Application.h"

#include <GLFW/glfw3.h>

namespace Viper
{

	Input *Input::instance = new WindowsInput();

	bool WindowsInput::isKeyPressedImpl(int keyCode)
	{
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		int state = glfwGetKey(window, keyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::isMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::getMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { float(xpos), float(ypos) };
	}

	float WindowsInput::getMouseXImpl()
	{
		auto[x, y] = this->getMousePositionImpl();
		return x;
	}

	float WindowsInput::getMouseYImpl()
	{
		auto[x, y] = this->getMousePositionImpl();
		return y;
	}

}