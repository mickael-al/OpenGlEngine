#ifndef __COLLISION_WRAPER__
#define __COLLISION_WRAPER__

#include <glm/glm.hpp>

namespace Ge
{
	class CollisionBody;
	class CommandQueue;
	class CollisionShape;
	class CollisionWraper
	{
	public:
		CollisionWraper(CollisionBody* cb, CommandQueue* queue);
        void Build(CollisionShape* shape);
        void setPosition(glm::vec3 pos);
        void setRotation(glm::quat rot);
        void setEulerAngles(glm::vec3 eul);
        void setScale(glm::vec3 scale);
        glm::vec3 getPosition() const;
        glm::quat getRotation() const;
        glm::vec3 getEulerAngles();
        glm::vec3 getScale() const;
        void setFriction(float friction);
        void setRestitution(float restitution);
        bool IsInitialized();
        const CollisionShape* getCollisionShape() const;
        void mapMemory();
    private:
        friend class PhysicsWraper;
        CollisionBody* getCollisionBody() const;
    private:
        CollisionBody* m_cb;
        CommandQueue* m_queue;
	};
}
#endif //!__COLLISION_WRAPER__