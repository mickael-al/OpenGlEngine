#ifndef __DEBUG_PATH_FINDING__
#define __DEBUG_PATH_FINDING__

#include "GameEngine.hpp"
#include "Player.hpp"
#include "CharacterPathFinder.hpp"


class DebugPathFinding : public Behaviour
{
public:
	DebugPathFinding(CharacterPathFinder * cpf, Player * p);
	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
private:
	ptrClass m_pc;
	ShapeBuffer* m_cube;
	Model* m_modelStart;
	Model* m_modelEnd;
	CharacterPathFinder* m_cpf;
	Player* m_p;
};


#endif //!__DEBUG_PATH_FINDING__