#ifndef __SCENE_VIEW__
#define __SCENE_VIEW__

#include "EngineHeader.hpp"
#include "Scene.hpp"

class SceneView final : Scene
{
public:
	void load();
	void unload();
private:
	const ptrClass* m_pc;
};

#endif//!__SCENE_VIEW__