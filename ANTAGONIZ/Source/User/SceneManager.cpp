#include "SceneManager.hpp"
#include "GameEngine.hpp"

namespace Ge
{
	void SceneManager::addScene(std::string name, Scene * s)
	{
		if (entryScene == "")
		{
			entryScene = name;
		}
		m_mapScene.emplace(name, s);
	}

	void SceneManager::removeScene(std::string name)
	{
		if (entryScene == name)
		{
			entryScene = "";
			if (m_mapScene.size() > 1)
			{
				entryScene = m_mapScene.begin()->first;
			}
		}
		m_mapScene.erase(name);
	}

	void SceneManager::loadEntryScene()
	{
		if (m_mapScene.size() > 0 && entryScene != "")
		{
			currentScene = m_mapScene[entryScene];
			currentScene->load();
		}
	}

	void SceneManager::release()
	{
		if (currentScene != nullptr)
		{
			currentScene->unload();
		}
		m_mapScene.clear();
		entryScene = "";
	}

	void SceneManager::loadScene(std::string name)
	{
		if (m_mapScene.count(name) == 0)
		{
			return;
		}
		if (currentScene != nullptr)
		{
			currentScene->unload();
		}
		currentScene = m_mapScene[name];
		currentScene->load();
		Time* t = GameEngine::getPtrClass().time;
		t->fixedUpdateTime();
		t->updateTime();
	}
}