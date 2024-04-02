#ifndef __ENGINE_GAME_ENGINE___
#define __ENGINE_GAME_ENGINE___

#include "Initializer.hpp"
#include "PointeurClass.hpp"
#include "Debug.hpp"
#include "RenderingEngine.hpp"
#include "SettingManager.hpp"
#include "Time.hpp"
#include "InputManager.hpp"

namespace Ge
{
    class GameEngine final : Initializer
    {
    public:
		GameEngine();		
        bool initialize();
        void release();
        void start();
        static const ptrClass& getPtrClass();
	private:
        void update();
    private:
		static ptrClass m_pointeurClass;
        RenderingEngine m_renderingEngine;
        Debug m_debug;
        SettingManager m_settingManager;        
        Time m_time;
        InputManager m_inputManager;
		BehaviourManager m_behaviourManager;
		SceneManager m_sceneManager;
        float m_lag = 0.0f;
    };
}

#endif //__ENGINE_GAME_ENGINE___