#ifndef __RIGID_WRAPER__
#define __RIGID_WRAPER__

#include <glm/glm.hpp>

namespace Ge
{
	class RigidBody;
    class CommandQueue;
    class CollisionShape;
	class RigidWraper
	{
	public:
		RigidWraper(RigidBody * rb, CommandQueue* queue);
        void Translate(glm::vec3 const& translate);
        void setPosition(glm::vec3 pos);
        void setRotation(glm::quat rot);
        void setEulerAngles(glm::vec3 eul);
        glm::vec3 getPosition() const;
        glm::quat getRotation() const;
        glm::vec3 getEulerAngles();
        glm::vec3 getGravity() const;
        void SetMass(float mass);
        void setGravity(glm::vec3 const& gravity);
        void SetRestitution(float coef);
        void SetLinearVelocity(const glm::vec3 velocity);
        void BuildPhysics(CollisionShape* shape, bool hasInertia = true);
        bool IsInitialized();
        glm::vec3 GetLinearVelocity();
        void ApplyImpulse(glm::vec3 impulse, glm::vec3 real_pos);
        void forceActivationState(int newState);
        void SetSleepingThreshold(float linear, float angular);
        void lockRotation();
        void onGUI();
    private:
        friend class PhysicsWraper;
        RigidBody* getRigidBody() const;
	private:
		RigidBody* m_rb;
        CommandQueue* m_queue;
	};
}

#endif //!__RIGID_WRAPER__