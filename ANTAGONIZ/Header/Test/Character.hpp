#ifndef __CHARACTER__
#define __CHARACTER__

#include "GameEngine.hpp"

class Character : public Behaviour
{
public:
	Character(ShapeBuffer * quad,int indexGp, Textures * texture, Materials* shadowMat,int sizeX = 4,int sizeY = 4,glm::vec3 position = glm::vec3(0));
	~Character();
	virtual void start();
	void fixedUpdate();
	virtual void update();
	virtual void stop();
	void onGUI();
	void stepAnimation(int type);
	void idelAnimation(int type);
	Model* getModel();
	RigidBody* getRigidBody();
protected:
	ptrClass m_pc;
	ShapeBuffer* m_quad;
	Textures* m_texture;
	Materials* m_shadowMat;
	Materials* m_material;
	int m_index_gp;
	Model* m_model;
	Model* m_modelShadow;
	RigidBody* m_rb;
	float m_step = 1;
	float m_anim = 0;
	glm::vec2 m_dividAnimeTexture;
	glm::vec3 m_raycast;
	glm::vec3 m_position_base;
};

#endif //!__CHARACTER__