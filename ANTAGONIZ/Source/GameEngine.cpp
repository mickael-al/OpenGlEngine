#include "GameEngine.hpp"

namespace Ge
{
	ptrClass GameEngine::m_pointeurClass;
	GameEngine::GameEngine()
	{
		m_pointeurClass.settingManager = &m_settingManager;
		m_pointeurClass.inputManager = &m_inputManager;
		m_pointeurClass.time = &m_time;
		m_pointeurClass.behaviourManager = &m_behaviourManager;
		m_pointeurClass.sceneManager = &m_sceneManager;
	}

	const ptrClass & GameEngine::getPtrClass()
	{
		return m_pointeurClass;
	}

    bool GameEngine::initialize()
    {
        if(!m_renderingEngine.initialize(&m_pointeurClass))
        {
            Debug::INITFAILED("RenderingEngine");
            return false;
        }
        return true;
    }

    void GameEngine::release()
    {        
        m_sceneManager.release();
        m_behaviourManager.release();
		m_renderingEngine.release();
        m_time.release();
        m_inputManager.release();
        Debug::RELEASESUCCESS("GameEngine");
    }

    void GameEngine::start()
    {        
        m_time.startTime();
        Debug::Info("Moteur Start");
        m_sceneManager.loadEntryScene();
        GameEngine::update();        
    }
    
    void GameEngine::update()
    {
        while (!glfwWindowShouldClose(str_window))/*gestion d'evenement lier a la fermeture de la fenetre via la croix */
		{
			glfwPollEvents();/*event de recuperation*/
            m_time.fixedUpdateTime();        
			m_lag += m_time.getFixedDeltaTime();
			m_behaviourManager.fixedUpdate();
			if (m_lag >= 1.0/m_settingManager.getFps())
			{
				m_time.updateTime();                
				m_inputManager.updateAxis();
				m_behaviourManager.update();
				m_renderingEngine.drawFrame();
				m_lag -= 1.0/m_settingManager.getFps();
			}
		}
    }
}