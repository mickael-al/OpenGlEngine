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
#include "PhysicsEngine.hpp"
#include "PhysicsWraper.hpp"
#include "SoundManager.hpp"
#include "Editor.hpp"
#include "PathManager.hpp"
#include <Windows.h>
#include "CommandQueue.hpp"

namespace Ge
{
	ptrClass Engine::m_pointeurClass;
	Engine::Engine()
	{
		m_queue = new CommandQueue();
		m_graphicsDataMisc = new GraphicsDataMisc();
		m_pointeurClass.settingManager = m_settingManager = new SettingManager();
		m_pointeurClass.inputManager = m_inputManager = new InputManager();
		m_pointeurClass.time = m_time = new Time();
		m_pointeurClass.behaviourManager = m_behaviourManager = new BehaviourManager();
		m_pointeurClass.sceneManager = m_sceneManager = new SceneManager();
		m_physicsEngine = new PhysicsEngine();
		m_pointeurClass.physicsEngine = m_physicsEngineWraper = new PhysicsWraper(m_physicsEngine,m_queue);
		m_pointeurClass.soundManager = m_soundManager = new SoundManager();
		m_editor = new Editor();
		m_renderingEngine = new RenderingEngine(m_graphicsDataMisc);		
		m_pointeurClass.gdm = m_graphicsDataMisc;
	}

	Engine::~Engine()
	{
		delete m_queue;
		delete m_settingManager;
		delete m_inputManager;
		delete m_time;
		delete m_behaviourManager;
		delete m_sceneManager;
		delete m_graphicsDataMisc;
		delete m_physicsEngine;
		delete m_physicsEngineWraper;
		delete m_soundManager;
		delete m_editor;
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
		PathManager::initDirectory();
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
		if (!m_soundManager->initialize())
		{
			Debug::INITFAILED("SoundManager");
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
		m_soundManager->release();
		m_fixedThreadRuning = false;
		m_fixedThread.join();
		if (m_settingManager->getEditor())
		{
			m_behaviourManager->removeBehaviour(m_editor,true);
		}
        Debug::RELEASESUCCESS("Engine");
    }

	int deltaTime(int previous, int offset) 
	{
		return (clock() - previous) + offset;
	}

	void updatePhysique(glm::vec3 gravity,PhysicsEngine * pe, std::atomic<bool>* fixedThreadRuning,CommandQueue * queue)
	{		
		unsigned long m_lastTime = GetTickCount64();
		unsigned long currentTime;
		float deltaTime;
		pe->Initialize(gravity, queue);
		while (*fixedThreadRuning)
		{
			currentTime = GetTickCount64();
			deltaTime = (currentTime - m_lastTime) / 1000.0f;
			m_lastTime = currentTime;
			pe->Update(deltaTime);
		}
		pe->Shutdown();
	}

    void Engine::start()
    {        
        m_time->startTime();		
		m_fixedThreadRuning = true;
		m_fixedThread = std::thread(updatePhysique, m_settingManager->getGravity(), m_physicsEngine, &m_fixedThreadRuning, m_queue);
        Debug::Info("Moteur Start");
		if (m_settingManager->getEditor())
		{
			m_behaviourManager->addBehaviour(m_editor);
		}
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