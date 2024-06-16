#ifndef __ENGINE_GRAPHIC_OBJECT__
#define __ENGINE_GRAPHIC_OBJECT__

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vector>
#include <string> 
#include "Transform.hpp"

class Component;

namespace Ge
{
	class GObject
	{
	public:
		GObject(bool addList = true);
		~GObject();
		void setName(std::string nom);
		std::string * getName();
		virtual glm::mat4 getModelMatrix() const;
		virtual void setPosition(glm::vec3 pos);
		virtual void setRotation(glm::quat rot);
		virtual void setLocalPosition(glm::vec3 pos);
		virtual void setLocalRotation(glm::quat rot);
		virtual void setEulerAngles(glm::vec3 eul);
		virtual void setScale(glm::vec3 scale);
		virtual void setTarget(glm::vec3 target);
		virtual glm::vec3 getPosition() const;
		virtual glm::quat getRotation() const;
		virtual glm::vec3 getLocalPosition() const;
		virtual glm::quat getLocalRotation() const;
		virtual glm::vec3 getEulerAngles();
		virtual glm::vec3 getDirection() const;
		virtual glm::vec3 getScale() const;	
		virtual void mapMemory() = 0;
		void updateLocal();
		void updateGlobal();
		glm::vec3 transformDirectionAxeX();
		glm::vec3 transformDirectionAxeY();
		glm::vec3 transformDirectionAxeZ();
		glm::vec3 transformDirectionAxe(glm::vec3 dir);
		Transform globalTransform(const glm::vec3& localPosition, const glm::quat& localRotation, const glm::vec3& localScale, const glm::vec3& parentPosition, const glm::quat& parentRotation, const glm::vec3& parentScale);
		Transform globalTransform(const glm::vec3& localPosition, const glm::quat& localRotation, const GObject* parent);		
		Transform globalTransform(const glm::vec3& localPosition, const glm::vec3& localEuler, const glm::vec3& localScale, const glm::vec3& parentPosition, const glm::quat& parentRotation, const glm::vec3& parentScale);
		Transform globalTransform(const glm::vec3& localPosition, const glm::vec3& localEuler, const GObject* parent);
		void applyTransform(const glm::vec3& localPosition, const glm::quat& localRotation, const GObject* parent);
		void applyTransform(const glm::vec3& localPosition, const glm::vec3& localEuler, const GObject* parent);
		void applyLocalTransform(const glm::vec3& globalPosition, const glm::quat& globalRotation, const GObject* parent);
		void addComponent(Component * c);
		void removeComponent(Component * c);
		static std::vector<GObject *> GetGObjects();
		static std::vector<GObject*> FindObjectsWithTag(std::string tag);
		std::vector<size_t> & getTag();
		void addTag(std::string n);		
		void removeTag(std::string n);
		void setParent(GObject* obj);
		GObject* getParent() const;
		const std::vector<GObject*> & getChilds() const;
		virtual void onGUI();
	protected:
		static std::vector<GObject *> s_gobjects;	
		std::vector<size_t> m_tag;
		GObject* m_parent = nullptr;
		std::vector<GObject*> m_child;
		std::string m_nom;
		Transform m_transform{};
		LocalTransform m_localTransform{};
		glm::vec3 m_eulerAngles;//only UI
		std::vector<Component *> m_component;
		bool m_addList = true;
	};
}

#endif //!__ENGINE_GRAPHIC_OBJECT__