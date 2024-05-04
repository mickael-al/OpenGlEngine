#include "SceneView.hpp"
#include "Debug.hpp"
#include "Materials.hpp"
#include "Model.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"

void SceneView::load()
{
	m_pc = &Engine::getPtrClass();
	ShapeBuffer * sb = m_pc->modelManager->allocateBuffer("../Asset/Model/cube.obj");
	Model * m = m_pc->modelManager->createModel(sb);
	Model* m2 = m_pc->modelManager->createModel(sb);
	Model* m3 = m_pc->modelManager->createModel(sb);
	Model* m4 = m_pc->modelManager->createModel(sb);
	Model* m5 = m_pc->modelManager->createModel(sb);
	Materials* mat = m_pc->materialManager->createMaterial();
	mat->setMetallic(0.5f);
	mat->setRoughness(0.5f);
	mat->setNormal(0.25f);
	m->setMaterial(mat);
	Textures* n = m_pc->textureManager->createTexture("../Asset/Texture/n.png");
	mat->setNormalTexture(n);
	mat->setTilling(glm::vec2(20, 20));
	m2->setPosition(glm::vec3(3, 3, 0));
	m->setScale(glm::vec3(20, 1, 20));
	m2->setScale(glm::vec3(1, 5, 1));

	m3->setPosition(glm::vec3(-2, 2, 0));
	m4->setPosition(glm::vec3(0, 2, 2));
	m5->setPosition(glm::vec3(0, 2, -2));

	m3->setScale(glm::vec3(1, 5, 1));
	m4->setScale(glm::vec3(1, 5, 1));
	m5->setScale(glm::vec3(1, 5, 1));

	m_tc = new TestCube(m2);
	m_pc->behaviourManager->addBehaviour(m_tc);
}

void SceneView::unload()
{
	m_pc->behaviourManager->removeBehaviour(m_tc,true);
	m_tc->stop();
	delete m_tc;
}