#ifndef __BEHAVIOUR_TEST__
#define __BEHAVIOUR_TEST__

#include "Behaviour.hpp"
#include "Debug.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "RigidWraper.hpp"
#include "CollisionWraper.hpp"

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
    CollisionShape* m_cb2;
    std::vector<RigidWraper*> m_rw;
    std::vector<Model*> m_model;
    ShapeBuffer* m_shape;
    int countObject = 20;
};

REGISTER(Behaviour, BehaviourTest);

#endif //!__BEHAVIOUR_TEST__