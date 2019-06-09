#include "vpch.h"
#include <Viper.h>

class GameLayer : public Viper::Layer
{
public:
	GameLayer()
		: Layer("Game layer")
	{
	}

	void onEvent(Viper::Event &e) override
	{
		if (Viper::Input::isKeyPressed(V_KEY_TAB))
			V_INFO("TAB IS PRESSED!");
	}

};

class Game : public Viper::Application
{
public:
	Game()
	{
		pushLayer(new GameLayer());
	}

	~Game(){}


};

Viper::Application *Viper::createApplication()
{
	return new Game();
}