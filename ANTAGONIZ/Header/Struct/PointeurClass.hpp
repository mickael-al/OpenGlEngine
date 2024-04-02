#ifndef __ENGINE_POINTEUR_CLASS__
#define __ENGINE_POINTEUR_CLASS__

#include "SettingManager.hpp"
#include "InputManager.hpp"
#include "Time.hpp"
#include "ModelManager.hpp"
#include "TextureManager.hpp"
#include "LightManager.hpp"
#include "CameraManager.hpp"
#include "MaterialManager.hpp"
#include "BehaviourManager.hpp"
#include "SceneManager.hpp"
#include "GraphiquePipelineManager.hpp"
#include "SkyboxManager.hpp"

using namespace Ge;

struct ptrClass
{
	SettingManager * settingManager;
    InputManager * inputManager;	
	Time * time;
	ModelManager * modelManager;
	LightManager * lightManager;
	CameraManager * cameraManager;
	BehaviourManager * behaviourManager;
	SceneManager * sceneManager;
	SkyboxManager * skyboxManager;
};

#endif //__ENGINE_POINTEUR_CLASS__