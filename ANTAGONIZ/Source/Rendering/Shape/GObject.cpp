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

		m_localTransform.position = glm::vec3(0.0f);
		m_localTransform.rotation = glm::quat(1, 0, 0, 0);
		m_localEulerAngles = glm::vec3(0.0f);
		m_localTransform.scale = glm::vec3(1.0f);
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
			m_localEulerAngles = glm::degrees(glm::eulerAngles(m_transform.rotation));
			updateGlobal();
		}
		else
		{
			setRotation(rot);
		}
	}

	void GObject::setLocalEuler(glm::vec3 eul)
	{
		if (m_parent != nullptr)
		{
			m_localEulerAngles = eul;
			m_localTransform.rotation = glm::quat(glm::radians(eul));
			updateGlobal();
		}
		else
		{
			setEulerAngles(eul);
		}
	}

	void GObject::setLocalScale(glm::vec3 scale)
	{
		if (m_parent != nullptr)
		{
			m_localTransform.scale = scale;			
			updateGlobal();
		}
		else
		{
			setScale(scale);
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
		glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
		m_transform.rotation = glm::rotation(forward, direction);
		m_eulerAngles = GObject::getEulerAngles();
		updateLocal();
	}

	glm::vec3 GObject::getLocalPosition() const
	{
		return m_localTransform.position;
	}

	glm::quat GObject::getLocalRotation() const
	{
		return m_localTransform.rotation;
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
			glm::mat4 pworld = m_parent->getModelMatrix();
			glm::mat4 world = getModelMatrix();
			glm::mat4 local = glm::inverse(pworld) * world;
			m_localTransform = decomposeMatrix(local);
			m_localEulerAngles = glm::degrees(glm::eulerAngles(m_localTransform.rotation));
		}
		else
		{
			m_localTransform = m_transform;
			m_localEulerAngles = m_eulerAngles;
		}
		updateGlobal();
	}

	void GObject::updateGlobal()
	{
		if (m_parent)
		{
			glm::mat4 pworld = m_parent->getModelMatrix();
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_localTransform.position);
			glm::mat4 rotationMatrix = glm::toMat4(glm::quat(m_localTransform.rotation));
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_localTransform.scale);
			glm::mat4 local = translationMatrix * rotationMatrix * scaleMatrix;
			glm::mat4 world = pworld * local;
			m_transform = decomposeMatrix(world);
		}
		mapMemory();

		for (GObject* child : m_child)
		{
			child->updateGlobal();
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

	Transform GObject::decomposeMatrix(const glm::mat4& m)
	{
		Transform t;

		t.position = glm::vec3(m[3]);

		t.scale.x = glm::length(glm::vec3(m[0]));
		t.scale.y = glm::length(glm::vec3(m[1]));
		t.scale.z = glm::length(glm::vec3(m[2]));

		glm::mat3 rotMat;
		rotMat[0] = glm::vec3(m[0]) / t.scale.x;
		rotMat[1] = glm::vec3(m[1]) / t.scale.y;
		rotMat[2] = glm::vec3(m[2]) / t.scale.z;

		t.rotation = glm::quat_cast(rotMat);

		return t;
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

		if (ImGui::DragFloat3("Scale", (float*)&m_transform.scale, 0.2f))
		{
			setScale(m_transform.scale);
		}

		if (ImGui::DragFloat3("LocalPosition", (float*)&m_localTransform.position, 0.2f))
		{
			setLocalPosition(m_localTransform.position);
		}

		if (ImGui::DragFloat3("LocalEulerAngles", (float*)&m_localEulerAngles, 0.2f))
		{
			setLocalEuler(m_localEulerAngles);
		}

		if (ImGui::DragFloat3("LocalScale", (float*)&m_localTransform.scale, 0.2f))
		{
			setLocalScale(m_localTransform.scale);
		}

		for (Component *comp : m_component)
		{
			comp->onGUI();
		}
	}
}