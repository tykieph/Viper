#include "vpch.h"
#include "LayerStack.h"

namespace Viper
{

	LayerStack::LayerStack()
	{
		this->layerInsert = this->layers.begin();
	}


	LayerStack::~LayerStack()
	{
		for (Layer *layer : this->layers)
			delete layer;
	}


	void LayerStack::pushLayer(Layer *layer)
	{
		this->layerInsert = this->layers.emplace(this->layerInsert, layer);
	}

	void LayerStack::pushOverlay(Layer *overlay)
	{
		this->layers.emplace_back(overlay);
	}


	void LayerStack::popLayer(Layer *layer)
	{
		auto it = std::find(this->layers.begin(), this->layers.end(), layer);

		if (it != this->layers.end())
		{
			this->layers.erase(it);
			this->layerInsert--;
		}
	}


	void LayerStack::popOverlay(Layer *overlay)
	{
		auto it = std::find(this->layers.begin(), this->layers.end(), overlay);

		if (it != this->layers.end())
			this->layers.erase(it);
	}

}