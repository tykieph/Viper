#pragma once


namespace Viper
{

	enum class RendererAPI
	{
		None = 0, Vulkan = 1
	};

	class Renderer
	{
	public:
		inline static RendererAPI getAPI() { return rendererAPI; }

	private:
		static RendererAPI rendererAPI;
	};

}