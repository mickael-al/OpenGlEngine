#ifndef __DEBUG_COLLISION__
#define __DEBUG_COLLISION__

#include "GameEngine.hpp"

class DebugCollision : public Behaviour
{
public:
	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
private:
	ptrClass m_pc;
	ShapeBuffer* m_cube;
	Model* m_model;
};


#endif //!__DEBUG_COLLISION__