#pragma once



namespace Viper
{

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() {};
		virtual void init() = 0;
		virtual void swapBuffers() = 0;

		// testing
		virtual void updateVertices(std::pair<float, float> v1, std::pair<float, float> v2, std::pair<float, float> v3) = 0;

	};

}