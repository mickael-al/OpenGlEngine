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
#include "Hud.hpp"
#include "BehaviourManager.hpp"
#include "SceneManager.hpp"
#include "GraphiquePipelineManager.hpp"
#include "PhysicsWraper.hpp"
#include "SoundManager.hpp"
#include "PostProcess.hpp"
#include "Skybox.hpp"

using namespace Ge;
namespace Ge
{
	class RenderingEngine;
};

struct ptrClass
{
	SettingManager * settingManager;
    InputManager * inputManager;	
	Time * time;
	ModelManager * modelManager;
	TextureManager * textureManager;
	LightManager * lightManager;
	CameraManager * cameraManager;
	MaterialManager * materialManager;
	BehaviourManager * behaviourManager;
	SceneManager * sceneManager;
	PhysicsWraper* physicsEngine;
	SoundManager* soundManager;
	Hud * hud;
	GraphiquePipelineManager * graphiquePipelineManager;
	RenderingEngine * renderingEngine;
	PostProcess* postProcess;
	Skybox * skybox;
	GraphicsDataMisc* gdm;
};

#endif //__ENGINE_POINTEUR_CLASS__