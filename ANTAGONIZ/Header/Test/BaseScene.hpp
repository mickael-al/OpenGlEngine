#ifndef __BASE_SCENE__
#define __BASE_SCENE__

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "Player.hpp"
#include <vector>

class BaseScene : public Scene
{
public:
	void load();
	void unload();
private:
	ptrClass m_ptrc;
};

#endif //!__BASE_SCENE__