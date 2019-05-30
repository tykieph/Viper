#pragma once

#ifdef V_PLATFORM_WINDOWS

	// somewhere(in client) we need to define function - Viper::Application *Viper::createApplication()
	extern Viper::Application *Viper::createApplication();

	int main(int argc, char **argv)
	{
		Viper::Log::init();

		V_CORE_INFO("{0}", sizeof(int));

		Viper::Application *app = Viper::createApplication(); 
		app->run();
		delete app;
	}

#else
	#error Viper supports only Windows
#endif