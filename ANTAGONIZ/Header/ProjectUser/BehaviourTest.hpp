#ifndef __BEHAVIOUR_TEST__
#define __BEHAVIOUR_TEST__

#include "Behaviour.hpp"
#include "Debug.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
using namespace Ge;

class BehaviourTest : public Behaviour
{
public:
    void start();
    void fixedUpdate();
    void update();
    void stop();
    void onGUI();
private:
    const ptrClass * m_pc;
    CollisionShape* m_cb;
    CollisionBody* m_cbBody;
};

REGISTER(Behaviour, BehaviourTest);

#endif //!__BEHAVIOUR_TEST__