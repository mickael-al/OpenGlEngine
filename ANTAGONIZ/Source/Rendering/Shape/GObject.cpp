#include "GObject.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/common.hpp"
#include "Component.hpp"
#include "imgui-cmake/Header/imgui.h"
#include "Transform.hpp"
#include "Debug.hpp"
#include <algorithm>
#include <functional>

namespace Ge
{
	std::vector<GObject *> GObject::s_gobjects;
	GObject::GObject(bool addList)
	{
		m_nom = "NoName";
		m_transform.position = glm::vec3(0.0f);
		m_transform.rotation = glm::quat(1, 0, 0, 0);
		m_eulerAngles = glm::vec3(0.0f);
		m_transform.scale = glm::vec3(1.0f);		
		m_addList = addList;
		if (m_addList)
		{
			s_gobjects.push_back(this);
		}
	}

	GObject::~GObject()
	{
		if (m_addList)
		{
			s_gobjects.erase(std::remove(s_gobjects.begin(), s_gobjects.end(), this), s_gobjects.end());
		}
	}

	std::vector<GObject *> GObject::GetGObjects()
	{
		return s_gobjects;
	}

	std::vector<size_t> & GObject::getTag()
	{
		return m_tag;
	}

	std::vector<GObject*> GObject::FindObjectsWithTag(std::string tag)
	{
		size_t hashValue = std::hash<std::string>{}(tag);
		std::vector<GObject*> objs;
		for (int i = 0; i < s_gobjects.size(); i++)
		{
			const std::vector<size_t>& m_t = s_gobjects[i]->getTag();
			for (int j = 0; j < m_t.size(); j++)
			{
				if (m_t[j] == hashValue)
				{
					j = m_t.size();			
					objs.push_back(s_gobjects[i]);
				}
			}
		}
		return objs;
	}

	void GObject::addTag(std::string n)
	{
		size_t hashValue = std::hash<std::string>{}(n);
		m_tag.push_back(hashValue);
	}

	void GObject::removeTag(std::string n)
	{
		size_t hashValue = std::hash<std::string>{}(n);
		m_tag.erase(std::remove(m_tag.begin(), m_tag.end(), hashValue), m_tag.end());
	}

	void GObject::setParent(GObject* obj)
	{
		if (m_parent != nullptr)
		{			
			m_parent->m_child.erase(std::remove(m_parent->m_child.begin(), m_parent->m_child.end(), this), m_parent->m_child.end());
		}
		m_parent = obj;
		if (m_parent != nullptr)
		{
			updateLocal();
			m_parent->m_child.push_back(this);
		}
	}

	GObject* GObject::getParent() const
	{
		return m_parent;
	}

	const std::vector<GObject*>& GObject::getChilds() const
	{
		return m_child;
	}

	void GObject::setName(std::string name)
	{
		m_nom = name;
	}

	std::string *GObject::getName()
	{
		return &m_nom;
	}

	void GObject::setPosition(glm::vec3 pos)
	{
		m_transform.position = pos;
		updateLocal();		
	}

	void GObject::setEulerAngles(glm::vec3 euler)
	{
		m_eulerAngles = euler;
		m_transform.rotation = glm::quat(glm::radians(euler));
		updateLocal();		
	}

	void GObject::setRotation(glm::quat rot)
	{		
		m_transform.rotation = rot;
		m_eulerAngles = GObject::getEulerAngles();
		updateLocal();
	}

	void GObject::setLocalPosition(glm::vec3 pos)
	{
		if (m_parent != nullptr)
		{
			m_localTransform.position = pos;
			applyTransform(m_localTransform.position, m_localTransform.rotation, m_parent);
			updateGlobal();
		}
		else
		{
			setPosition(pos);
		}		
	}

	void GObject::setLocalRotation(glm::quat rot)
	{
		if (m_parent != nullptr)
		{
			m_localTransform.rotation = rot;
			applyTransform(m_localTransform.position, m_localTransform.rotation, m_parent);
			updateGlobal();
		}
		else
		{
			setRotation(rot);
		}
	}

	void GObject::setScale(glm::vec3 scale)
	{
		m_transform.scale = scale;
		updateLocal();
	}

	void GObject::setTarget(glm::vec3 target)
	{
		glm::vec3 direction = glm::normalize(target - m_transform.position);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 rotationMatrix = glm::lookAt(m_transform.position, target, up);		
		m_transform.rotation = glm::quat_cast(rotationMatrix);
		updateLocal();
	}

	glm::vec3 GObject::getLocalPosition() const
	{
		return m_parent != nullptr ? m_localTransform.position : glm::vec3(0,0,0);
	}

	glm::quat GObject::getLocalRotation() const
	{
		return m_parent != nullptr ? m_localTransform.rotation : glm::quat(0, 0, 0,0);		
	}

	glm::vec3 GObject::getPosition() const
	{
		return m_transform.position;
	}

	glm::quat GObject::getRotation() const
	{
		return m_transform.rotation;
	}

	glm::vec3 GObject::getEulerAngles()
	{
		return glm::degrees(glm::eulerAngles(m_transform.rotation));
	}

	glm::vec3 GObject::getDirection() const
	{
		return glm::rotate(m_transform.rotation, glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 GObject::getScale() const
	{
		return m_transform.scale;
	}

	void GObject::updateLocal()
	{
		if (m_parent != nullptr)
		{
			applyLocalTransform(m_transform.position, m_transform.rotation, m_parent);
		}
		mapMemory();
		updateGlobal();		
	}

	void GObject::updateGlobal()
	{
		for (int i = 0; i < m_child.size(); i++)
		{
			m_child[i]->applyTransform(m_child[i]->getLocalPosition(), m_child[i]->getLocalRotation(), this);
			m_child[i]->updateGlobal();
		}
	}

	glm::vec3 GObject::transformDirectionAxeX()
	{
		return glm::rotate(m_transform.rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
	}

	glm::vec3 GObject::transformDirectionAxeY()
	{
		return glm::rotate(m_transform.rotation, glm::vec3(0.0f, -1.0f, 0.0f));
	}

	glm::vec3 GObject::transformDirectionAxeZ()
	{
		return glm::rotate(m_transform.rotation, glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 GObject::transformDirectionAxe(glm::vec3 dir)
	{
		return glm::rotate(m_transform.rotation, dir);
	}

	Transform GObject::globalTransform(const glm::vec3& localPosition, const glm::quat& localRotation, const glm::vec3& localScale,const glm::vec3& parentPosition, const glm::quat& parentRotation, const glm::vec3& parentScale)
	{		
		Transform globalTransform;
		glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), localPosition) * glm::mat4_cast(localRotation) * glm::scale(glm::mat4(1.0f), localScale);
		glm::mat4 parentMatrix = glm::translate(glm::mat4(1.0f), parentPosition) * glm::mat4_cast(parentRotation) * glm::scale(glm::mat4(1.0f), parentScale);	
		glm::mat4 globalMatrix = parentMatrix * localMatrix;
		globalTransform.position = glm::vec3(globalMatrix[3][0], globalMatrix[3][1], globalMatrix[3][2]);
		globalTransform.rotation = glm::quat_cast(globalMatrix);
		globalTransform.scale = localScale;
		return globalTransform;
	}

	Transform GObject::globalTransform(const glm::vec3& localPosition, const glm::quat& localRotation, const GObject* parent)
	{
		Transform globalTransform;
		glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), localPosition) * glm::mat4_cast(localRotation) * glm::scale(glm::mat4(1.0f), m_transform.scale);
		glm::mat4 parentMatrix = glm::translate(glm::mat4(1.0f), parent->m_transform.position) * glm::mat4_cast(parent->m_transform.rotation) * glm::scale(glm::mat4(1.0f), parent->m_transform.scale);
		glm::mat4 globalMatrix = parentMatrix * localMatrix;
		globalTransform.position = glm::vec3(globalMatrix[3][0], globalMatrix[3][1], globalMatrix[3][2]);
		globalTransform.rotation = glm::quat_cast(globalMatrix);
		globalTransform.scale = m_transform.scale;
		return globalTransform;
	}

	void GObject::applyTransform(const glm::vec3& localPosition, const glm::quat& localRotation,const GObject * parent)
	{
		glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), localPosition) * glm::mat4_cast(localRotation);// *glm::scale(glm::mat4(1.0f), m_transform.scale);
		glm::mat4 parentMatrix = glm::translate(glm::mat4(1.0f), parent->m_transform.position) * glm::mat4_cast(parent->m_transform.rotation);// *glm::scale(glm::mat4(1.0f), parent->m_transform.scale);
		glm::mat4 globalMatrix = parentMatrix * localMatrix;
		m_transform.position = glm::vec3(globalMatrix[3][0], globalMatrix[3][1], globalMatrix[3][2]);
		m_transform.rotation = glm::quat_cast(globalMatrix);
		m_eulerAngles = getEulerAngles();
		mapMemory();
	}

	void GObject::applyLocalTransform(const glm::vec3& globalPosition, const glm::quat& globalRotation, const GObject* parent)
	{
		glm::mat4 globalMatrix = glm::translate(glm::mat4(1.0f), globalPosition) * glm::mat4_cast(globalRotation);
		glm::mat4 parentInverseMatrix = glm::inverse(parent->getModelMatrix());

		m_localTransform.position = glm::vec3(parentInverseMatrix * glm::vec4(globalPosition, 1.0f));

		glm::quat parentRotation = parent->getRotation();
		if (glm::length(parentRotation) > std::numeric_limits<float>::epsilon()) 
		{
			m_localTransform.rotation = glm::inverse(parentRotation) * globalRotation;
		}
		else 
		{
			m_localTransform.rotation = globalRotation;
		}
	}

	Transform GObject::globalTransform(const glm::vec3& localPosition, const glm::vec3& localEuler, const glm::vec3& localScale, const glm::vec3& parentPosition, const glm::quat& parentRotation, const glm::vec3& parentScale)
	{
		return globalTransform(localPosition, glm::quat(glm::radians(localEuler)), localScale, parentPosition, parentRotation, parentScale);
	}

	Transform GObject::globalTransform(const glm::vec3& localPosition, const glm::vec3& localEuler, const GObject* parent)
	{
		return globalTransform(localPosition, glm::quat(glm::radians(localEuler)), parent);
	}

	void GObject::applyTransform(const glm::vec3& localPosition, const glm::vec3& localEuler, const GObject* parent)
	{
		applyTransform(localPosition, glm::quat(glm::radians(localEuler)),parent);
	}

	void GObject::addComponent(Component *c)
	{
		m_component.push_back(c);
	}

	void GObject::removeComponent(Component *c)
	{
		m_component.erase(std::remove(m_component.begin(), m_component.end(), c), m_component.end());
	}

	glm::mat4 GObject::getModelMatrix() const
	{
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_transform.position);
		glm::mat4 rotationMatrix = glm::toMat4(glm::quat(m_transform.rotation));
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_transform.scale);
		return translationMatrix * rotationMatrix * scaleMatrix;
	}

	int CSize(std::string n)
	{
		const char* charPtr = n.c_str();
		for (int i = 0; i < n.size(); i++)
		{
			if (charPtr[i] == '\0')
			{
				return i;
			}
		}
		return n.size();
	}

	void GObject::onGUI()
	{
		const char* charPtr = m_nom.c_str();
		if (ImGui::InputText("Name", const_cast<char*>(charPtr), m_nom.capacity() + 1, ImGuiInputTextFlags_AllowTabInput))
		{
			m_nom.resize(CSize(m_nom) + 1);
		}
		if (ImGui::DragFloat3("Position", (float *)&m_transform.position, 0.2f))
		{
			setPosition(m_transform.position);
		}

		if (ImGui::DragFloat3("EulerAngles", (float*)&m_eulerAngles, 0.2f))
		{
			setEulerAngles(m_eulerAngles);
		}

		if (ImGui::DragFloat4("Rotation", (float *)&m_transform.rotation, 0.2f))
		{
			setRotation(m_transform.rotation);
		}

		if (ImGui::DragFloat3("Scale", (float *)&m_transform.scale, 0.2f))
		{
			setScale(m_transform.scale);
		}

		for (Component *comp : m_component)
		{
			comp->onGUI();
		}
	}
}