#ifndef __PATH_FINDING_SCENE__
#define __PATH_FINDING_SCENE__

#include "Engine.hpp"
#include "PathFinder.hpp"
#include "AStar.hpp"
#include "Vec3AStar.hpp"
#include "Behaviour.hpp"
#include "PointeurClass.hpp"

class PathFindingScene : public Behaviour
{
public:
	PathFindingScene(glm::vec3 position, glm::vec3 zoneSize, glm::vec3 pointCount, std::string path, float liasonPercent = 0.1f);
	~PathFindingScene();
	void generateMap();
	void debugPoint();
	void SortTree();
	void saveToFile();
	void loadFromFile();
	void FillGraph();
	void help();
	unsigned int nearPointIndex(glm::vec3 target);
	bool pathFinding(glm::vec3* startPosition, glm::vec3* endPosition, std::vector<Vec3AStar*>* path);

	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
	static PathFindingScene* getInstance();
private:
	void filterClosePoints();
	void generateNeighbors();
private:
	static PathFindingScene* instance;
	ptrClass m_pc;
	glm::vec3 m_position;
	glm::vec3 m_zoneSize;
	glm::vec3 m_pointCount;

	float m_liasonPercent;
	PathFinder<Vec3AStar> * m_finder;
	Vec3AStar* m_vec3Astars;
	std::vector<glm::vec3> m_points;
	std::vector<std::vector<unsigned int>> m_neighbors;

	std::vector<Model*> m_models;
	std::vector<Materials*> m_materials;
	std::vector<ShapeBuffer*> m_shapes;

	glm::vec3 m_boundsMin;
	glm::vec3 m_boundsMax;
	std::vector<uint64_t> m_morton;
	std::string m_path;
};

#endif //!__PATH_FINDING_SCENE__