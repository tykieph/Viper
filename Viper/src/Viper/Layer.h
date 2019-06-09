#pragma once

#include "Viper/Core.h"
#include "Viper/Events/Event.h"

namespace Viper
{

	class VIPER_API Layer
	{
	public:
		Layer(const std::string &name = "Layer");
		virtual ~Layer();

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate() {}
		virtual void onEvent(Event &event) {};

		inline const std::string &getName() { return this->debugName; }

	protected:
		std::string debugName;

	};

}