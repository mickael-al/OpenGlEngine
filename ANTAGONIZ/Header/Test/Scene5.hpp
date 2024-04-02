#ifndef __SCENE_5__
#define __SCENE_5__

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "InitData.hpp"
#include "MPGrassland.hpp"
#include "PrefabLoader.hpp"
#include "CharacterPathFinder.hpp"
#include "PathFindingScene.hpp"
#include <random>
#include "Particle.hpp"
#include "WaveFunctionCollapse.hpp"

struct PosID
{
	glm::vec3 pos;
	int id;
	glm::vec3 scale;
	glm::vec3 euler;
	bool collider;
	glm::uvec2 random;
	glm::vec3 RandomPos;
	glm::vec3 RandomScale;
	glm::vec3 RandomEuler;
	int seed;
};

class Scene5 : public Scene, public Behaviour
{
public:
	Scene5(InitData* id);
	void load();
	void unload();

	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
	void SpawnWfcPrefab(std::vector<PosID> p, std::vector<LoadPrefab*> lp, glm::vec3 globalPos);
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
	WaveFunctionCollapse<20,20> * wfc;
};

#endif __SCENE_5__