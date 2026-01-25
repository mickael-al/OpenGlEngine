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
		virtual void setEulerAngles(glm::vec3 eul);
		virtual void setScale(glm::vec3 scale);
		virtual void setTarget(glm::vec3 target);
		virtual void setLocalPosition(glm::vec3 pos);
		virtual void setLocalRotation(glm::quat rot);
		virtual void setLocalEuler(glm::vec3 rot);
		virtual void setLocalScale(glm::vec3 scale);
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
		Transform decomposeMatrix(const glm::mat4& m);
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
		Transform m_localTransform{};
		glm::vec3 m_eulerAngles;//only UI
		glm::vec3 m_localEulerAngles;//only UI
		std::vector<Component *> m_component;
		bool m_addList = true;
	};
}

#endif //!__ENGINE_GRAPHIC_OBJECT__