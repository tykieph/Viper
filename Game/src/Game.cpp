#include "vpch.h"
#include <Viper.h>

class Game : public Viper::Application
{
public:
	Game(){}
	~Game(){}

	void onEvent(Viper::Event& event) override
	{
		if (Viper::Input::isKeyPressed(V_KEY_TAB))
			V_INFO("TAB IS PRESSED!");
	}

};

Viper::Application *Viper::createApplication()
{
	return new Game();
}