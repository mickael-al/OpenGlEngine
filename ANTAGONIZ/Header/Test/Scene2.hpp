#ifndef __SCENE_2__
#define __SCENE_2__

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "InitData.hpp"
#include "MPDungeon.hpp"
#include "PrefabLoader.hpp"
#include "CharacterPathFinder.hpp"
#include "PathFindingScene.hpp"
#include <random>
#include "Particle.hpp"

class Scene2 : public Scene, public Behaviour
{
public:
	Scene2(InitData* id);
	void load();
	void unload();

	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
private:
	PrefabLoader* m_prefabLoader;
	std::vector<Textures*> m_textures;
	std::vector<Particle*> m_particles;
	std::vector<Block*> m_blocks;
	std::vector<Character*> m_characters;
	std::vector<SoundBuffer*> m_SoundScene;
	std::vector<AudioSource*> m_AudioScene;
	std::vector<CollisionBody*> m_CollisionScene;
	std::vector<Lights*> m_LightsScene;
	ptrClass m_pc;
	InitData* m_id;
	std::random_device m_dev;
	std::mt19937 m_rng;
	std::uniform_real_distribution<float> m_dist;
};

#endif __SCENE_2__