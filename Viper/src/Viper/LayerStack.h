#pragma once

#include "Viper/Core.h"
#include "Viper/Layer.h"

#include <vector>

namespace Viper
{

	class VIPER_API LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void pushLayer(Layer *layer);
		void pushOverlay(Layer *overlay);
		void popLayer(Layer *layer);
		void popOverlay(Layer *overlay);

		std::vector<Layer *>::iterator begin() { return this->layers.begin(); }
		std::vector<Layer *>::iterator end() { return this->layers.end(); }

	private:
		std::vector<Layer *> layers;
		std::vector<Layer *>::iterator layerInsert;
	};

}