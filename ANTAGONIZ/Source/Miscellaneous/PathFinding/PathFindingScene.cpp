#include "PathFindingScene.hpp"
#include "Materials.hpp"
#include "Model.hpp"

PathFindingScene* PathFindingScene::instance = nullptr;

PathFindingScene::PathFindingScene(glm::vec3 position, glm::vec3 zoneSize, glm::vec3 pointCount, std::string path, float liasonPercent)
{
	m_pc = Engine::getPtrClass();
	m_position = position;
	m_zoneSize = zoneSize;
	m_pointCount = pointCount;
	m_liasonPercent = liasonPercent;
	m_finder = nullptr;
	m_octree = nullptr;
	m_boundsMax = glm::vec3(0);
	m_boundsMin = glm::vec3(0);
	m_vec3Astars = nullptr;
	m_path = path;
	instance = this;
}

PathFindingScene::~PathFindingScene()
{
	if (m_finder != nullptr)
	{
		delete m_finder;
		delete[] m_vec3Astars;
		m_finder = nullptr;
	}
	if (m_octree != nullptr)
	{
		delete m_octree;
		m_octree = nullptr;
	}
	m_points.clear();
	m_neighbors.clear();	
	instance = nullptr;
}

uint64_t morton3D_64(uint64_t x, uint64_t y, uint64_t z) 
{
	x = (x | (x << 32)) & 0x1F00000000FFFFL;
	x = (x | (x << 16)) & 0x1F0000FF0000FFL;
	x = (x | (x << 8)) & 0x100F00F00F00F00FL;
	x = (x | (x << 4)) & 0x10C30C30C30C30C3L;
	x = (x | (x << 2)) & 0x1249249249249249L;

	y = (y | (y << 32)) & 0x1F00000000FFFFL;
	y = (y | (y << 16)) & 0x1F0000FF0000FFL;
	y = (y | (y << 8)) & 0x100F00F00F00F00FL;
	y = (y | (y << 4)) & 0x10C30C30C30C30C3L;
	y = (y | (y << 2)) & 0x1249249249249249L;

	z = (z | (z << 32)) & 0x1F00000000FFFFL;
	z = (z | (z << 16)) & 0x1F0000FF0000FFL;
	z = (z | (z << 8)) & 0x100F00F00F00F00FL;
	z = (z | (z << 4)) & 0x10C30C30C30C30C3L;
	z = (z | (z << 2)) & 0x1249249249249249L;

	return x | (y << 1) | (z << 2);
}

uint64_t vec3ToMorton_64(const glm::vec3& vec, const glm::vec3& boundsMin, const glm::vec3& boundsMax)
{
	const uint64_t MAX = (1ULL << 21) - 1; // 21 bits par coordonnée

	// Normalisation et conversion en entiers
	uint64_t x = static_cast<uint64_t>(
		std::clamp((vec.x - boundsMin.x) / (boundsMax.x - boundsMin.x), 0.0f, 1.0f) * MAX
		);
	uint64_t y = static_cast<uint64_t>(
		std::clamp((vec.y - boundsMin.y) / (boundsMax.y - boundsMin.y), 0.0f, 1.0f) * MAX
		);
	uint64_t z = static_cast<uint64_t>(
		std::clamp((vec.z - boundsMin.z) / (boundsMax.z - boundsMin.z), 0.0f, 1.0f) * MAX
		);

	return morton3D_64(x, y, z);
}

// Fonction pour comparer deux glm::vec3 selon le code de Morton
bool mortonCompare(const glm::vec3& a, const glm::vec3& b, const glm::vec3& boundsMin, const glm::vec3& boundsMax)
{		
	return vec3ToMorton_64(a, boundsMin, boundsMax) < vec3ToMorton_64(b, boundsMin, boundsMax);
}

void calculateBounds(const std::vector<glm::vec3>& points, glm::vec3& boundsMin, glm::vec3& boundsMax) 
{
	if (points.empty()) 
	{
		boundsMin = glm::vec3(0.0f);
		boundsMax = glm::vec3(0.0f);
		return;
	}

	boundsMin = boundsMax = points[0];

	for (const glm::vec3& point : points) 
	{	
		boundsMin.x = std::min(boundsMin.x, point.x);
		boundsMin.y = std::min(boundsMin.y, point.y);
		boundsMin.z = std::min(boundsMin.z, point.z);

		boundsMax.x = std::max(boundsMax.x, point.x);
		boundsMax.y = std::max(boundsMax.y, point.y);
		boundsMax.z = std::max(boundsMax.z, point.z);
	}
	const float eps = 0.0001f;
	if (abs(boundsMax.x - boundsMin.x) <= eps) { boundsMin.x -= eps; boundsMax.x += eps; }
	if (abs(boundsMax.y - boundsMin.y) <= eps) { boundsMin.y -= eps; boundsMax.y += eps; }
	if (abs(boundsMax.z - boundsMin.z) <= eps) { boundsMin.z -= eps; boundsMax.z += eps; }
}

void PathFindingScene::SortTree()
{
	calculateBounds(m_points, m_boundsMin, m_boundsMax);

	std::sort(m_points.begin(), m_points.end(), [&](const glm::vec3& a, const glm::vec3& b) 
		{
		return mortonCompare(a, b, m_boundsMin,m_boundsMax);
		});
	m_morton.resize(m_points.size());
	for (int i = 0; i < m_points.size(); i++)
	{
		m_morton[i] = vec3ToMorton_64(m_points[i],m_boundsMin,m_boundsMax);
	}		
}


void PathFindingScene::generateMap()
{
	glm::vec3 pointSpacing = m_zoneSize / m_pointCount;	
	float x, y, z;
	glm::vec3 startPosition, endPosition, hitPosition;	
	glm::vec3 zoneSizeDivid = m_zoneSize / 2.0f;	

	m_points.reserve((m_pointCount.x * m_pointCount.y * m_pointCount.z) / 4);
	Debug::Log("PathFinding generate");
	for (x = 0; x < m_zoneSize.x; x += pointSpacing.x)
	{
		for (y = 0; y < m_zoneSize.y; y += pointSpacing.y)
		{
			for (z = 0; z < m_zoneSize.z; z += pointSpacing.z)
			{
				endPosition.x = startPosition.x = x + m_position.x - zoneSizeDivid.x;
				endPosition.y = startPosition.y = y + m_position.y - zoneSizeDivid.y;
				endPosition.z = startPosition.z = z + m_position.z - zoneSizeDivid.z;
				endPosition.y -= 1000.0f;
				if (!m_pc.physicsEngine->isPointInsideCollision(&startPosition))
				{
					if (m_pc.physicsEngine->raycast(&startPosition, &endPosition, &hitPosition))
					{
						m_points.push_back(hitPosition);
					}
				}
			}
		}
	}
	Debug::Log("PathFinding Points: %d", m_points.size());
	filterClosePoints();
	Debug::Log("PathFinding FilterClosePoints : %d", m_points.size());	
	SortTree();
	generateNeighbors();
	FillGraph();
}

void PathFindingScene::debugPoint()
{
	std::vector<float> positions;
	std::vector<float> texcoords;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	// ====== POINTS ======
	float s = 0.05f; // demi-taille du cube local autour du point

	for (const glm::vec3& p : m_points)
	{
		unsigned int baseIndex = positions.size() / 3;

		// Chaque point 3 quads : XY, YZ, XZ

		// --- Quad XY (face "sol")
		glm::vec3 xy[4] = {
			p + glm::vec3(-s, -s, 0),
			p + glm::vec3(s, -s, 0),
			p + glm::vec3(s, s, 0),
			p + glm::vec3(-s, s, 0)
		};
		glm::vec3 n_xy(0, 0, 1);

		// --- Quad YZ (face latérale)
		glm::vec3 yz[4] = {
			p + glm::vec3(0, -s, -s),
			p + glm::vec3(0, s, -s),
			p + glm::vec3(0, s, s),
			p + glm::vec3(0, -s, s)
		};
		glm::vec3 n_yz(1, 0, 0);

		// --- Quad XZ (autre face latérale)
		glm::vec3 xz[4] = {
			p + glm::vec3(-s, 0, -s),
			p + glm::vec3(s, 0, -s),
			p + glm::vec3(s, 0, s),
			p + glm::vec3(-s, 0, s)
		};
		glm::vec3 n_xz(0, 1, 0);

		auto pushQuad = [&](glm::vec3 q[4], const glm::vec3& n) {
			unsigned int idx = positions.size() / 3;
			for (int i = 0; i < 4; ++i) {
				positions.insert(positions.end(), { q[i].x, q[i].y, q[i].z });
				texcoords.insert(texcoords.end(), { (i == 1 || i == 2) ? 1.0f : 0.0f, (i >= 2) ? 1.0f : 0.0f });
				normals.insert(normals.end(), { n.x, n.y, n.z });
			}
			indices.insert(indices.end(), {
				idx, idx + 1, idx + 2,
				idx, idx + 2, idx + 3
				});
		};

		pushQuad(xy, n_xy);
		pushQuad(yz, n_yz);
		pushQuad(xz, n_xz);
	}

	ShapeBuffer* meshPoints = m_pc.modelManager->allocateBuffer(
		positions.data(), texcoords.data(), normals.data(), indices.data(),
		positions.size()/3, indices.size());


	// ====== SEGMENTS ======
	positions.clear();
	texcoords.clear();
	normals.clear();
	indices.clear();

	float halfWidth = 0.02f;

	for (int i = 0; i < m_points.size(); i++)
	{
		glm::vec3 p1 = m_points[i];
		for (unsigned int neighborIndex : m_neighbors[i])
		{
			if (neighborIndex <= i) { continue; } // éviter doublons}

			glm::vec3 p2 = m_points[neighborIndex];
			glm::vec3 dir = glm::normalize(p2 - p1);

			// 2 axes perpendiculaires à dir pour croiser les quads
			glm::vec3 ref(0, 1, 0);
			if (fabs(glm::dot(ref, dir)) > 0.9f) ref = glm::vec3(1, 0, 0);

			glm::vec3 side1 = glm::normalize(glm::cross(ref, dir)) * halfWidth;
			glm::vec3 side2 = glm::normalize(glm::cross(dir, side1)) * halfWidth;

			// Fonction utilitaire pour un quad entre p1 et p2
			auto pushSegmentQuad = [&](const glm::vec3& offset, const glm::vec3& normal) {
				unsigned int baseIndex = positions.size() / 3;

				glm::vec3 a = p1 - offset;
				glm::vec3 b = p1 + offset;
				glm::vec3 c = p2 + offset;
				glm::vec3 d = p2 - offset;

				positions.insert(positions.end(), {
					a.x,a.y,a.z,
					b.x,b.y,b.z,
					c.x,c.y,c.z,
					d.x,d.y,d.z
					});

				texcoords.insert(texcoords.end(), { 0,0, 1,0, 1,1, 0,1 });

				for (int j = 0; j < 4; ++j)
					normals.insert(normals.end(), { normal.x, normal.y, normal.z });

				indices.insert(indices.end(), {
					baseIndex, baseIndex + 1, baseIndex + 2,
					baseIndex, baseIndex + 2, baseIndex + 3
					});
			};

			// Quad 1
			pushSegmentQuad(side1, glm::normalize(glm::cross(dir, side1)));
			// Quad 2 (croisé)
			pushSegmentQuad(side2, glm::normalize(glm::cross(dir, side2)));
		}
	}

	ShapeBuffer* meshSegments = m_pc.modelManager->allocateBuffer(
		positions.data(), texcoords.data(), normals.data(), indices.data(),
		positions.size()/3, indices.size());

	m_shapes.push_back(meshPoints);
	m_shapes.push_back(meshSegments);
	Materials* material = m_pc.materialManager->createMaterial();
	Materials* materialEdge = m_pc.materialManager->createMaterial();
	m_materials.push_back(material);
	m_materials.push_back(materialEdge);
	material->setColor(glm::vec4(0, 1, 0, 1));
	material->setMetallic(0.0f);
	material->setRoughness(1.0f);

	materialEdge->setColor(glm::vec4(0, 0, 1, 1));
	materialEdge->setMetallic(0.0f);
	materialEdge->setRoughness(1.0f);

	GraphiquePipeline* gp = m_pc.graphiquePipelineManager->createPipeline("../Asset/Shader/opaque.fs.glsl", "../Asset/Shader/opaque.vs.glsl", false, true, false, 2);
	material->setPipeline(gp);
	materialEdge->setPipeline(gp);
	Model* m = m_pc.modelManager->createModel(meshPoints);
	m->setMaterial(material);
	m_models.push_back(m);

	m = m_pc.modelManager->createModel(meshSegments);
	m->setMaterial(materialEdge);
	m_models.push_back(m);
}


void PathFindingScene::saveToFile() 
{
	std::ofstream outFile(m_path, std::ios::binary);
	if (!outFile.is_open()) 
	{
		Debug::Error("Failed to open file for writing: %s", m_path.c_str());
		return;
	}

	// Write header
	outFile.write(reinterpret_cast<char*>(&m_position), sizeof(glm::vec3));
	outFile.write(reinterpret_cast<char*>(&m_zoneSize), sizeof(glm::vec3));
	outFile.write(reinterpret_cast<char*>(&m_pointCount), sizeof(glm::vec3));
	outFile.write(reinterpret_cast<char*>(&m_liasonPercent), sizeof(float));

	// Write number of points
	int size_points = m_points.size();
	outFile.write(reinterpret_cast<char*>(&size_points), sizeof(int));

	// Write points
	for (const glm::vec3& point : m_points) 
	{
		outFile.write(reinterpret_cast<const char*>(&point), sizeof(glm::vec3));
	}

	// Write neighbors
	for (const std::vector<unsigned int>& neighbors : m_neighbors) 
	{
		int size_neighbors = neighbors.size();
		outFile.write(reinterpret_cast<const char*>(&size_neighbors), sizeof(int));
		for (unsigned int neighbor : neighbors) 
		{
			outFile.write(reinterpret_cast<const char*>(&neighbor), sizeof(unsigned int));
		}
	}

	outFile.write(reinterpret_cast<char*>(&m_boundsMin), sizeof(glm::vec3));
	outFile.write(reinterpret_cast<char*>(&m_boundsMax), sizeof(glm::vec3));

	// Write number of morton
	int size_morton = m_morton.size();
	outFile.write(reinterpret_cast<char*>(&size_morton), sizeof(int));

	for (const uint64_t& morton : m_morton)
	{
		outFile.write(reinterpret_cast<const char*>(&morton), sizeof(uint64_t));
	}

	outFile.close();
}

void PathFindingScene::loadFromFile() 
{
	std::ifstream inFile(m_path, std::ios::binary);
	if (!inFile.is_open()) 
	{
		Debug::Error("Failed to open file for reading: %s", m_path.c_str());
		return;
	}

	// header
	inFile.read(reinterpret_cast<char*>(&m_position), sizeof(glm::vec3));
	inFile.read(reinterpret_cast<char*>(&m_zoneSize), sizeof(glm::vec3));
	inFile.read(reinterpret_cast<char*>(&m_pointCount), sizeof(glm::vec3));
	inFile.read(reinterpret_cast<char*>(&m_liasonPercent), sizeof(float));

	// Read number of points
	int size_points = 0;
	inFile.read(reinterpret_cast<char*>(&size_points), sizeof(int));

	// Read points
	m_points.resize(size_points);
	for (glm::vec3& point : m_points) 
	{
		inFile.read(reinterpret_cast<char*>(&point), sizeof(glm::vec3));
	}

	// Read neighbors
	m_neighbors.resize(size_points);
	for (std::vector<unsigned int>& neighbors : m_neighbors) 
	{
		int size_neighbors = 0;
		inFile.read(reinterpret_cast<char*>(&size_neighbors), sizeof(int));
		neighbors.resize(size_neighbors);
		for (unsigned int& neighbor : neighbors) 
		{
			inFile.read(reinterpret_cast<char*>(&neighbor), sizeof(unsigned int));
		}
	}

	inFile.read(reinterpret_cast<char*>(&m_boundsMin), sizeof(glm::vec3));
	inFile.read(reinterpret_cast<char*>(&m_boundsMax), sizeof(glm::vec3));

	// Write number of morton
	int size_morton = 0;
	inFile.read(reinterpret_cast<char*>(&size_morton), sizeof(int));

	m_morton.resize(size_morton);

	for (uint64_t& morton : m_morton)
	{
		inFile.read(reinterpret_cast<char*>(&morton), sizeof(uint64_t));
	}

	inFile.close();
	FillGraph();
}

void PathFindingScene::help()
{
	Debug::Log("PathFindingScene generateMap -> P");
	Debug::Log("PathFindingScene debugPoint -> O");
	Debug::Log("PathFindingScene loadFromFile -> L");
	Debug::Log("PathFindingScene saveToFile -> K");
}

void PathFindingScene::FillGraph()
{
	if (m_finder != nullptr)
	{
		delete m_finder;
		delete[] m_vec3Astars;
	}
	if (m_octree != nullptr)
	{
		delete m_octree;
		m_octree = nullptr;
	}

	m_finder = new PathFinder<Vec3AStar>();
	m_vec3Astars = new Vec3AStar[m_points.size()];
	for (int i = 0; i < m_points.size(); i++)
	{
		m_vec3Astars[i].position = m_points[i];
	}
	for (int i = 0; i < m_points.size(); i++)
	{
		for (unsigned int neighborIndex : m_neighbors[i])
		{			
			Node * n = (Node*)(m_vec3Astars+i);		
			n->addChild((Node*)(m_vec3Astars+neighborIndex), glm::distance(m_vec3Astars[i].position, m_vec3Astars[neighborIndex].position));			
		}
	}
	m_octree = new MortonOctree(m_points, m_morton, m_boundsMin, m_boundsMax, /*maxDepth=*/16, /*maxLeaf=*/8);
}

glm::vec3 PathFindingScene::nearPointIndexPos(const glm::vec3& target)
{
	unsigned int npi = nearPointIndex(target);
	return  m_vec3Astars[npi].position;
}

MortonOctree* PathFindingScene::getMortonOctree()
{
	return m_octree;
}

unsigned int PathFindingScene::nearPointIndex(const glm::vec3& target)
{
	return m_octree->findNearestIndex(target);

	/*float bestDist = std::numeric_limits<float>::max();
	unsigned int bestIndex = 0;

	for (unsigned int i = 0; i < m_points.size(); ++i)
	{
		float dist = glm::distance(m_points[i], target);
		if (dist < bestDist)
		{
			bestDist = dist;
			bestIndex = i;
		}
	}	

	return bestIndex;*/
	/*


	uint64_t targetMorton = vec3ToMorton_64(target, m_boundsMin, m_boundsMax);

	unsigned int left = 0;
	unsigned int right = static_cast<unsigned int>(m_morton.size()) - 1;
	unsigned int nearestIndex = 0;
	uint64_t nearestDistance = std::numeric_limits<uint64_t>::max();

	while (left <= right)
	{
		unsigned int middle = (left + right) / 2;
		uint64_t currentMorton = m_morton[middle];

		uint64_t currentDistance = (currentMorton > targetMorton) ? (currentMorton - targetMorton) : (targetMorton - currentMorton);

		if (currentDistance < nearestDistance)
		{
			nearestDistance = currentDistance;
			nearestIndex = middle;
		}

		if (currentMorton < targetMorton)
		{
			left = middle + 1;
		}
		else
		{
			if (middle == 0) { break; }// éviter overflow si right devient négatif
			right = middle - 1;
		}
	}

	unsigned int toCheck = 32;
	unsigned int start = (nearestIndex < toCheck) ? 0 : nearestIndex - toCheck;
	unsigned int end = std::min(static_cast<unsigned int>(m_points.size() - 1), nearestIndex + toCheck);

	float bestDist = std::numeric_limits<float>::max();
	unsigned int bestIndex = nearestIndex;

	for (unsigned int i = start; i <= end; ++i) 
	{
		float dist = glm::distance(m_points[i], target);
		if (dist < bestDist) 
		{
			bestDist = dist;
			bestIndex = i;
		}
	}

	return bestIndex;*/
}



bool PathFindingScene::pathFinding(glm::vec3 * startPosition, glm::vec3 *endPosition, std::vector<Vec3AStar*>* path)
{
	unsigned int startIndex = nearPointIndex(*startPosition);
	unsigned int endIndex = nearPointIndex(*endPosition);

	*startPosition = m_vec3Astars[startIndex].position;
	*endPosition = m_vec3Astars[endIndex].position;

	m_finder->setStart(m_vec3Astars[startIndex]);
	m_finder->setGoal(m_vec3Astars[endIndex]);
	
	if (m_finder->findPath<AStar>((*path)))
	{
		AStar::getInstance().clear();
		return true;
	}
	AStar::getInstance().clear();
	return false;
}

void PathFindingScene::filterClosePoints()
{
	for (int i = 0; i < m_points.size(); i++) 
	{
		for (int j = m_points.size() - 1; j > i; j--) 
		{
			if (glm::distance(m_points[i], m_points[j]) < 0.1f) 
			{
				m_points.erase(m_points.begin() + j);				
			}
		}
	}
}

void PathFindingScene::generateNeighbors()
{
	float maxDistance = (m_zoneSize.x / m_pointCount.x);
	maxDistance += maxDistance * m_liasonPercent;

	m_neighbors.resize(m_points.size());
	glm::vec3 hitp;
	for (std::size_t i = 0; i < m_points.size(); ++i) 
	{
		for (std::size_t j = 0; j < m_points.size(); ++j) 
		{
			if (i != j) 
			{
				float distance = glm::distance(m_points[i], m_points[j]);
				if (distance <= maxDistance) 
				{
					hitp = m_points[j];
					//m_pc.physicsEngine->raycast(&m_points[i], &m_points[j], &hitp);
					if (glm::distance(m_points[i], hitp) >= distance)
					{
						m_neighbors[i].push_back(j);
					}
					
				}
			}
		}
	}
}


void PathFindingScene::start()
{
	
}

void PathFindingScene::fixedUpdate()
{

}

void PathFindingScene::update()
{
	if (m_pc.inputManager->getKeyDown(GLFW_KEY_P))
	{
		generateMap();
	}
	if (m_pc.inputManager->getKeyDown(GLFW_KEY_O))
	{
		debugPoint();
		Debug::Log("debugPoint");
	}
	if (m_pc.inputManager->getKeyDown(GLFW_KEY_L))
	{
		loadFromFile();
		Debug::Log("load");
	}
	if (m_pc.inputManager->getKeyDown(GLFW_KEY_K))
	{
		saveToFile();
		Debug::Log("Save");
	}
}

void PathFindingScene::stop()
{
	for (int i = 0; i < m_shapes.size(); i++)
	{
		m_pc.modelManager->destroyBuffer(m_shapes[i]);
	}
	m_shapes.clear();
	for (int i = 0; i < m_models.size(); i++)
	{
		m_pc.modelManager->destroyModel(m_models[i]);
	}
	m_models.clear();
	for (int i = 0; i < m_materials.size(); i++)
	{
		m_pc.materialManager->destroyMaterial(m_materials[i]);
	}
	m_materials.clear();
}

PathFindingScene* PathFindingScene::getInstance()
{
	return instance;
}

void PathFindingScene::onGUI()
{

}