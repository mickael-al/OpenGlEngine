#include "EngineHeader.hpp"
#include <iostream>
#include "Version.hpp"
#include "PathManager.hpp"
#include <cstdlib>
using namespace Ge;

int main(int argc, char *argv[])
{
#ifdef _WIN32

#elif __linux__
	//!!! the working directory is changed in order to load shader models etc...
	if (argc >= 1)
	{
		PathManager::workDirectory(argv[0]);
	}
#endif

	Debug::Info("Moteur Graphique");
	Engine engine;
	ptrClass e = engine.getPtrClass();
	e.settingManager->setName("Antagoniz");
	e.settingManager->setWindowHeight(1017);
	e.settingManager->setWindowWidth(1920);
	e.settingManager->setEditor(true);
	e.settingManager->setEditorPath("../TestProject");
	Version vers;
	vers.majeur = 7;
	vers.mineur = 0;
	vers.patch = 0;
	e.settingManager->setVersion(vers);
	e.settingManager->setFps(120.0f);

	if (!engine.initialize())
	{
		Debug::Error("Erreur d'intialisation du moteur graphique");
		return -1;
	}

	try
	{
		engine.start();
	}
	catch (std::runtime_error& e)
	{
		Debug::Error("Exception : %s", e.what());
		return -1;
	}
	catch (std::bad_alloc& e)
	{
		Debug::Error("Exception : %s", e.what());
		return -1;
	}
	catch (const std::exception& e)
	{
		Debug::Error("Exception : %s", e.what());
		return -1;
	}
	engine.release();
	return 0;
}