#ifndef __CHARACTER_PATH_FINDER__
#define __CHARACTER_PATH_FINDER__

#include "Character.hpp"
#include "PathFindingScene.hpp"

class PathRoutine
{
public:
	glm::vec3 startPosition;
	glm::vec3 endPosition;
	std::vector<Vec3AStar*> path;
	int step;
};

class CharacterPathFinder : public Character
{
public:
	CharacterPathFinder(ShapeBuffer* quad, int indexGp, Textures* texture, Materials* shadowMat, PathFindingScene * pfs, int sizeX = 4, int sizeY = 4, glm::vec3 position = glm::vec3(0));
	bool addRoutinePath(glm::vec3 target);
	PathRoutine * routinePathDebug(glm::vec3 target);
	bool followPath();
	void clearAllPath();
	virtual void start() override;
	virtual void update() override;
private:
	Camera* m_cam;	

	float m_time_move_step = 0.22f;
	float m_speed = 40.0f;

	float m_move_step = 0.0f;

	glm::vec3 m_velocity = glm::vec3(0, 0, 0);
	float m_max_velocity = 20.0f;
	float frotement = 10.0f;

	int dir_anim = 0;
	glm::vec3 m_direction = glm::vec3(1, 0, 0);	

	std::vector<PathRoutine*> m_pathRoutines;
	float m_distanceToNextNode = 1.25f;
	PathFindingScene * m_pfs;
};

#endif //!__CHARACTER_PATH_FINDER__