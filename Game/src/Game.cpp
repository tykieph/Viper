#include "vpch.h"

#include <Viper.h>


class Game : public Viper::Application
{
public:
	Game(){}
	~Game(){}

	void run()
	{
		while (true)
		{
			std::cout << "testing\n";
		}
	}
};

Viper::Application *Viper::createApplication()
{
	return new Game();
}