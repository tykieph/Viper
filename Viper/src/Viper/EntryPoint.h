#pragma once

#ifdef V_PLATFORM_WINDOWS

	// somewhere(in client) we need to define function - Viper::Application *Viper::createApplication()
	extern Viper::Application *Viper::createApplication();

	int main(int argc, char **argv)
	{
		Viper::Application *app = Viper::createApplication(); 
		app->run();
		delete app;
	}

#else
	#error Viper supports only Windows
#endif