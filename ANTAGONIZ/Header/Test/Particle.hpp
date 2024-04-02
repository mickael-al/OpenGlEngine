#ifndef __PARTICLE__
#define __PARTICLE__

#include "GameEngine.hpp"

class Particle
{
public:
	Particle(ptrClass pc, int indexGp,ShapeBuffer* quad, Textures* texture, int sizeX, int sizeY,float time_step,glm::vec3 position,glm::vec3 scale);
	~Particle();
	Model* getModel() const;
	void Update(float dt);
	void Step();
private:
	ptrClass m_pc;
	Materials* m_mat;
	Model* m_model;
	float m_particle_update_time = 0.0f;
	float m_time_step_max;
	int m_step;
	int m_step_max;
	int m_sizeX;
	int m_sizeY;
};

#endif//!__PARTICLE__