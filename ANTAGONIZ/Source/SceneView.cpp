#include "SceneView.hpp"
#include "Debug.hpp"
#include "Materials.hpp"
#include "Model.hpp"
#include "DirectionalLight.hpp"
void SceneView::load()
{
	m_pc = &Engine::getPtrClass();
	ShapeBuffer * sb = m_pc->modelManager->allocateBuffer("../Asset/Model/cube.obj");
	Model * m = m_pc->modelManager->createModel(sb);
	Model* m2 = m_pc->modelManager->createModel(sb);
	Materials* mat = m_pc->materialManager->createMaterial();
	mat->setMetallic(0.7f);
	mat->setRoughness(0.2f);
	m->setMaterial(mat);
	Textures* n = m_pc->textureManager->createTexture("../Asset/Texture/n.png");
	//mat->setNormalTexture(n);
	mat->setTilling(glm::vec2(10, 10));
	m2->setMaterial(mat);
	m2->setPosition(glm::vec3(5, 5, 0));
	m->setScale(glm::vec3(10, 1, 10));
	//PointLight* pl = m_pc->lightManager->createPointLight(glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(1, 1, 1));
	DirectionalLight * dl = m_pc->lightManager->createDirectionalLight(glm::vec3(-45.0f, 90.0f, 0.0f), glm::vec3(1, 1, 1));	
	dl->setshadow(true);
}

void SceneView::unload()
{

}