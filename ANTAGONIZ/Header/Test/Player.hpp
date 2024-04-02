#ifndef __PLAYER__
#define __PLAYER__

#include "Character.hpp"
#include "PlayerMenu.hpp"

class Player : public Character
{
public:
	Player(ShapeBuffer* quad, int indexGp, Textures* texture, Materials* shadowMat, int sizeX = 4, int sizeY = 4, glm::vec3 position = glm::vec3(0));
	~Player();
	virtual void start() override;
	virtual void update() override;
	virtual void stop() override; 
private:
	PlayerMenu* m_pl;
	Camera* m_cam;
	float m_view_distance = 8.0f;
	float m_base_hauteur = 2.0f;
	float m_base_fov = 70.0f;
	float m_angle_y_cam = 0.0f;
	float m_angle_x_cam = 45.0f;
	float m_sensi_x = 0.25f;
	float m_sensi_y = 0.25f;
	glm::vec3 m_cam_hit;
	float m_vd = 0;
	glm::vec2 m_min_max_x_angle = glm::vec2(0.0f, 75.0f);

	float m_time_move_step = 0.15f;	
	float m_speed = 100.0f;	

	float m_move_step = 0.0f;	

	glm::vec3 m_velocity = glm::vec3(0,0,0);
	float m_max_velocity = 20.0f;
	float frotement = 10.0f;

	int dir_anim = 0;
	glm::vec3 direction = glm::vec3(0, 0, 0);
	glm::vec3 move_direction = glm::vec3(0, 0, 0);
	glm::vec3 move_input = glm::vec3(0, 0, 0);
};

#endif //!__PLAYER__