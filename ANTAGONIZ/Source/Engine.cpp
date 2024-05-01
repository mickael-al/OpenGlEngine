#include "Engine.hpp"
#include "GraphicsDataMisc.hpp"
#include "PointeurClass.hpp"
#include "Debug.hpp"
#include "RenderingEngine.hpp"
#include "InputManager.hpp"
#include "Time.hpp"
#include "BehaviourManager.hpp"
#include "SceneManager.hpp"
#include "Lights.hpp"

namespace Ge
{
	ptrClass Engine::m_pointeurClass;
	Engine::Engine()
	{
		m_graphicsDataMisc = new GraphicsDataMisc();
		m_pointeurClass.settingManager = m_settingManager = new SettingManager();
		m_pointeurClass.inputManager = m_inputManager = new InputManager();
		m_pointeurClass.time = m_time = new Time();
		m_pointeurClass.behaviourManager = m_behaviourManager = new BehaviourManager();
		m_pointeurClass.sceneManager = m_sceneManager = new SceneManager();
		m_renderingEngine = new RenderingEngine(m_graphicsDataMisc);
	}

	Engine::~Engine()
	{
		delete m_settingManager;
		delete m_inputManager;
		delete m_time;
		delete m_behaviourManager;
		delete m_sceneManager;
		delete m_graphicsDataMisc;
		delete m_renderingEngine;
	}

	const ptrClass & Engine::getPtrClass()
	{
		return m_pointeurClass;
	}

	const ptrClass * Engine::getPtrClassAddr()
	{
		return &m_pointeurClass;
	}

    bool Engine::initialize()
    {
        if(!m_renderingEngine->initialize(&m_pointeurClass))
        {
            Debug::INITFAILED("RenderingEngine");
            return false;
        }
        if(!m_inputManager->initialize(m_graphicsDataMisc))
        {
            Debug::INITFAILED("InputManager");
            return false;
        }
        return true;
    }

    void Engine::release()
    {        
        m_sceneManager->release();
        m_behaviourManager->release();
		m_renderingEngine->release();
        m_time->release();
        m_inputManager->release();
        Debug::RELEASESUCCESS("Engine");
    }

    void Engine::start()
    {        
        m_time->startTime();
        Debug::Info("Moteur Start");
        m_sceneManager->loadEntryScene();
        Engine::update();        
    }
    
    void Engine::update()
    {
        while (!glfwWindowShouldClose(m_graphicsDataMisc->str_window))/*gestion d'evenement lier a la fermeture de la fenetre via la croix */
		{
			glfwPollEvents();/*event de recuperation*/
            m_time->fixedUpdateTime();
			m_lag += m_time->getFixedDeltaTime();
			m_behaviourManager->fixedUpdate();
			if (m_lag >= 1.0/m_settingManager->getFps())
			{
				m_time->updateTime();
				m_inputManager->updateAxis();
				m_behaviourManager->update();
				m_renderingEngine->drawFrame();
				m_lag -= 1.0/m_settingManager->getFps();
			}
		}
    }
}