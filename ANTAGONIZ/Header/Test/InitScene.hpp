#ifndef __INIT_SCENE__
#define __INIT_SCENE__

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "InitData.hpp"
#include <vector>
#include "DebugCollision.hpp"

class InitScene : public Scene
{
public:
	void load();
	void unload();
private:
	ptrClass m_pc;		
	InitData initdata;
};

#endif //!__INIT_SCENE__