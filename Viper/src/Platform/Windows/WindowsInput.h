#pragma once

#include "Viper/Input.h"

namespace Viper
{

	class VIPER_API WindowsInput : public Input
	{
	protected:
		virtual bool isKeyPressedImpl(int keyCode) override;
		virtual bool isMouseButtonPressedImpl(int button) override;
		virtual std::pair<float, float> getMousePositionImpl() override;
		virtual float getMouseXImpl() override;
		virtual float getMouseYImpl() override;

	};

}