#ifndef __INIT_DATA__
#define __INIT_DATA__

#include "Player.hpp"
#include "GameEngine.hpp"

struct InitData
{
	Player* player;
	ShapeBuffer* quad;
	std::vector<Materials*> mats;
	std::vector<Textures*> textures;
	int pipeline_index_character;
	int pipeline_index_transparent;
	int pipeline_index_cull;
	int pipeline_index_unlit;
	int pipeline_index_particle;
	std::vector<Scene*> scene;
};

#endif //!__INIT_DATA__