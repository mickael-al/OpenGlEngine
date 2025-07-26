#ifndef __MUSCLE_WRAPER_HPP__
#define __MUSCLE_WRAPER_HPP__

#include <glm/glm.hpp>

namespace Ge
{
    class Muscle;
    class CommandQueue;
    class RigidWraper;
    class MuscleWraper
    {
    public:
        MuscleWraper(Muscle* mu, CommandQueue* queue);
        void setContraction(glm::vec3 m_value, float dt);
        glm::vec3 getContraction() const;
        void setWing(bool state);
        void reset();
    private:
        friend class PhysicsWraper;
        Muscle* getMuscle() const;
    private:
        Muscle * m_mu;
        CommandQueue* m_queue;
    };
}

#endif//!__MUSCLE_WRAPER_HPP__