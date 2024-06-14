#ifndef __ENGINE_ENGINE___
#define __ENGINE_ENGINE___

#include "Initializer.hpp"
#include <thread>
#include <atomic>
#include "glm/glm.hpp"

struct GraphicsDataMisc;
struct ptrClass;
namespace Ge
{
	class RenderingEngine;			
	class Debug;
	class SettingManager;
	class Time;
	class InputManager;
	class BehaviourManager;
	class SceneManager;
	class PhysicsEngine;
	class PhysicsWraper;
	class SoundManager;
	class Editor;
	class CommandQueue;
}
namespace Ge
{	
    class Engine final : Initializer
    {
    public:
		Engine();
		~Engine();		
        bool initialize();
        void release();
        void start();
		static const ptrClass & getPtrClass();
		static const ptrClass * getPtrClassAddr();
	private:
        void update();	
    private:
		static ptrClass m_pointeurClass;
		GraphicsDataMisc * m_graphicsDataMisc;
        RenderingEngine * m_renderingEngine;
        Debug * m_debug;
        SettingManager * m_settingManager;        
        Time * m_time;
        InputManager * m_inputManager;
		BehaviourManager * m_behaviourManager;
		SceneManager * m_sceneManager;
		PhysicsWraper* m_physicsEngineWraper;
		PhysicsEngine* m_physicsEngine;
		SoundManager* m_soundManager;
		Editor * m_editor;
		CommandQueue * m_queue;

		std::thread m_fixedThread;
		std::atomic<bool> m_fixedThreadRuning;
        float m_lag = 0.0f;
    };
}

#endif //!__ENGINE_GAME_ENGINE___