#pragma once

#include "Viper/Core.h"

namespace Viper
{

	class VIPER_API Input
	{
	public:
		inline static bool isKeyPressed(int keyCode) { return instance->isKeyPressedImpl(keyCode); };
		inline static bool isMouseButtonPressed(int button) { return instance->isMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> getMousePosition() { return instance->getMousePositionImpl(); }
		inline static float getMouseX() { return instance->getMouseXImpl(); }
		inline static float getMouseY() { return instance->getMouseYImpl(); }

	protected:
		// methods implementation
		virtual bool isKeyPressedImpl(int keyCode) = 0;
		virtual bool isMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> getMousePositionImpl() = 0;
		virtual float getMouseXImpl() = 0;
		virtual float getMouseYImpl() = 0;

	private:
		static Input *instance;
	};

}
