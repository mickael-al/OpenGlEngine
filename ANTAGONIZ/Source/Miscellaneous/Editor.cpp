/*
Trigger Warning a rushed code not optimized 
*/

#include "Editor.hpp"
#include "PointeurClass.hpp"
#include "Engine.hpp"
#include "CollisionWraper.hpp"
#include "PhysicsWraper.hpp"
#include "imgui-cmake/Header/imgui.h"
#include "PathManager.hpp"
#include "FolderDialog.hpp"
#include "Textures.hpp"
#include <sstream>
#include "GraphicsDataMisc.hpp"
#include "ModelManager.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "Materials.hpp"
#include "GraphiquePipeline.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "AudioSource.hpp"
#include "SoundBuffer.hpp"
#include "Empty.hpp"
#include "ImGuizmo.h"
#include "RenderingEngine.hpp"
#include "ShapeBufferBase.hpp"
#include <sys/stat.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "PhysicsEngine.hpp"
#include "CollisionBody.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "PathFindingScene.hpp"

using std::fstream;
using namespace Ge;
namespace fs = std::filesystem;

ImVec4 HSVtoRGB(float h, float s, float v) {
	int i;
	float f, p, q, t;
	if (s == 0.0f) {
		return ImVec4(v, v, v, 1.0f);
	}
	h /= 60.0f;
	i = (int)h;
	f = h - i;
	p = v * (1.0f - s);
	q = v * (1.0f - s * f);
	t = v * (1.0f - s * (1.0f - f));

	switch (i) {
	case 0:
		return ImVec4(v, t, p, 1.0f);
	case 1:
		return ImVec4(q, v, p, 1.0f);
	case 2:
		return ImVec4(p, v, t, 1.0f);
	case 3:
		return ImVec4(p, q, v, 1.0f);
	case 4:
		return ImVec4(t, p, v, 1.0f);
	default:
		return ImVec4(v, p, q, 1.0f);
	}
}

std::string removeFilename(const std::string& path) 
{
	// Trouver la dernière occurrence de '/' ou '\'
	size_t posSlash = path.find_last_of('/');
	size_t posBackslash = path.find_last_of('\\');

	// Trouver la position maximum des deux
	size_t pos = (posSlash == std::string::npos) ? posBackslash :
		(posBackslash == std::string::npos) ? posSlash :
		(posSlash > posBackslash ? posSlash : posBackslash);

	// Si aucun séparateur n'a été trouvé, retourner le chemin original
	if (pos == std::string::npos) {
		return path;
	}

	// Retourner la sous-chaîne jusqu'à la position du dernier séparateur
	return path.substr(0, pos);
}


std::string cropPath(std::string& str1, std::string& str2)
{
	size_t pos = str1.find(str2);
	if (pos != std::string::npos)
	{
		str1.erase(pos, str2.length());
	}
	return str1;
}

bool copyAndRenameDirectory(const std::string& sourceDir, const std::string& targetDir) 
{
	try 
	{
		if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir)) 
		{
			std::cerr << "Le dossier source n'existe pas ou n'est pas valide." << std::endl;
			return false;
		}

		if (fs::exists(targetDir))
		{
			std::cerr << "Le dossier cible existe déjà." << std::endl;
			return false;
		}

		fs::copy(sourceDir, targetDir, fs::copy_options::recursive);
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Une erreur s'est produite : " << e.what() << std::endl;
		return false;
	}
}

Editor* Editor::s_instance = nullptr;

void Editor::duplicateSceneObject(GObject* obj, GObject* parent)
{
	if (obj == nullptr)
	{
		return;
	}
	GObject * no = nullptr;
	if (Model* model = dynamic_cast<Model*>(obj))
	{
		Model* m = m_pc->modelManager->createModel(model->getShapeBuffer(), *model->getName());
		m->setPosition(model->getPosition());
		m->setRotation(model->getRotation());
		m->setScale(model->getScale());
		m->setMaterial(model->getMaterial());
		std::vector<size_t>& tags = model->getTag();
		std::vector<size_t>& mtags = m->getTag();
		mtags.reserve(tags.size());
		std::copy(tags.begin(), tags.end(), std::back_inserter(mtags));
		m->setParent(parent);
		no = m;
		if (scriptObject.find(model) != scriptObject.end())
		{
			std::vector<ScriptData> copy;
			copy.assign(scriptObject[model].begin(), scriptObject[model].end()); 
			scriptObject[m] = copy;
		}
		if (collisionObj.find(model) != collisionObj.end())
		{
			std::vector<CollisionData> copy;
			copy.assign(collisionObj[model].begin(), collisionObj[model].end());
			collisionObj[m] = copy;
		}
		m_currentSceneData->models.push_back(m);
	}
	if (Empty* empty = dynamic_cast<Empty*>(obj))
	{
		Empty* e = new Empty();
		e->setName(*empty->getName());
		e->setPosition(empty->getPosition());
		e->setRotation(empty->getRotation());
		e->setScale(empty->getScale());
		std::vector<size_t>& tags = empty->getTag();
		std::vector<size_t>& mtags = e->getTag();
		mtags.reserve(tags.size());
		std::copy(tags.begin(), tags.end(), std::back_inserter(mtags));
		e->setParent(parent);
		no = e;
		if (scriptObject.find(empty) != scriptObject.end())
		{
			std::vector<ScriptData> copy;
			copy.assign(scriptObject[empty].begin(), scriptObject[empty].end());
			scriptObject[e] = copy;
		}
		if (collisionObj.find(empty) != collisionObj.end())
		{
			std::vector<CollisionData> copy;
			copy.assign(collisionObj[empty].begin(), collisionObj[empty].end());
			collisionObj[e] = copy;
		}
		m_currentSceneData->empty.push_back(e);
	}
	if (DirectionalLight* dl = dynamic_cast<DirectionalLight*>(obj))
	{
		DirectionalLight* e = m_pc->lightManager->createDirectionalLight(dl->getEulerAngles(), dl->getColors(), *dl->getName());
		e->setPosition(dl->getPosition());
		e->setRotation(dl->getRotation());
		e->setScale(dl->getScale());
		e->setRange(dl->getRange());
		e->setshadow(dl->getshadow());
		e->setSpotAngle(dl->getSpotAngle());
		std::vector<size_t>& tags = dl->getTag();
		std::vector<size_t>& mtags = e->getTag();
		mtags.reserve(tags.size());
		std::copy(tags.begin(), tags.end(), std::back_inserter(mtags));
		e->setParent(parent);
		no = e;
		if (scriptObject.find(dl) != scriptObject.end())
		{
			std::vector<ScriptData> copy;
			copy.assign(scriptObject[dl].begin(), scriptObject[dl].end());
			scriptObject[e] = copy;
		}
		if (collisionObj.find(dl) != collisionObj.end())
		{
			std::vector<CollisionData> copy;
			copy.assign(collisionObj[dl].begin(), collisionObj[dl].end());
			collisionObj[e] = copy;
		}
		m_currentSceneData->dlight.push_back(e);
	}
	if (SpotLight* dl = dynamic_cast<SpotLight*>(obj))
	{
		SpotLight* e = m_pc->lightManager->createSpotLight(dl->getPosition(), dl->getColors(),dl->getEulerAngles(),dl->getSpotAngle(), *dl->getName());
		e->setPosition(dl->getPosition());
		e->setRotation(dl->getRotation());
		e->setScale(dl->getScale());
		e->setRange(dl->getRange());
		e->setshadow(dl->getshadow());
		e->setSpotAngle(dl->getSpotAngle());
		std::vector<size_t>& tags = dl->getTag();
		std::vector<size_t>& mtags = e->getTag();
		mtags.reserve(tags.size());
		std::copy(tags.begin(), tags.end(), std::back_inserter(mtags));
		e->setParent(parent);
		no = e;
		if (scriptObject.find(dl) != scriptObject.end())
		{
			std::vector<ScriptData> copy;
			copy.assign(scriptObject[dl].begin(), scriptObject[dl].end());
			scriptObject[e] = copy;
		}
		if (collisionObj.find(dl) != collisionObj.end())
		{
			std::vector<CollisionData> copy;
			copy.assign(collisionObj[dl].begin(), collisionObj[dl].end());
			collisionObj[e] = copy;
		}
		m_currentSceneData->slight.push_back(e);
	}
	if (PointLight* dl = dynamic_cast<PointLight*>(obj))
	{
		PointLight* e = m_pc->lightManager->createPointLight(dl->getPosition(), dl->getColors(), *dl->getName());
		e->setPosition(dl->getPosition());
		e->setRotation(dl->getRotation());
		e->setScale(dl->getScale());
		e->setRange(dl->getRange());
		e->setshadow(dl->getshadow());
		e->setSpotAngle(dl->getSpotAngle());
		std::vector<size_t>& tags = dl->getTag();
		std::vector<size_t>& mtags = e->getTag();
		mtags.reserve(tags.size());
		std::copy(tags.begin(), tags.end(), std::back_inserter(mtags));
		e->setParent(parent);
		no = e;
		if (scriptObject.find(dl) != scriptObject.end())
		{
			std::vector<ScriptData> copy;
			copy.assign(scriptObject[dl].begin(), scriptObject[dl].end());
			scriptObject[e] = copy;
		}
		if (collisionObj.find(dl) != collisionObj.end())
		{
			std::vector<CollisionData> copy;
			copy.assign(collisionObj[dl].begin(), collisionObj[dl].end());
			collisionObj[e] = copy;
		}
		m_currentSceneData->plight.push_back(e);
	}
	if (AudioSource* au = dynamic_cast<AudioSource*>(obj))
	{
		AudioSource* e = m_pc->soundManager->createSource(au->getSoundBuffer(), *au->getName());
		e->setPosition(au->getPosition());
		e->setRotation(au->getRotation());
		e->setScale(au->getScale());
		std::vector<size_t>& tags = au->getTag();
		std::vector<size_t>& mtags = e->getTag();
		mtags.reserve(tags.size());
		std::copy(tags.begin(), tags.end(), std::back_inserter(mtags));
		e->setParent(parent);
		no = e;
		if (scriptObject.find(au) != scriptObject.end())
		{
			std::vector<ScriptData> copy;
			copy.assign(scriptObject[au].begin(), scriptObject[au].end());
			scriptObject[e] = copy;
		}
		if (collisionObj.find(au) != collisionObj.end())
		{
			std::vector<CollisionData> copy;
			copy.assign(collisionObj[au].begin(), collisionObj[au].end());
			collisionObj[e] = copy;
		}
		m_currentSceneData->audio.push_back(e);
	}
	for (int c = 0; c < obj->getChilds().size(); c++)
	{
		duplicateSceneObject(obj->getChilds()[c], no);
	}
}


void removeExistTexture(Textures* texture, SceneData* sd, TextureManager* tm, GraphicsDataMisc* gdm)
{
	if (sd != nullptr)
	{
		if (texture == gdm->str_default_normal_texture || texture == gdm->str_default_texture)
		{
			return;
		}
		int count = 0;
		for (int i = 0; i < sd->materials.size(); i++)
		{
			if (sd->materials[i]->getAlbedoTexture() == texture) { count++; }
			if (sd->materials[i]->getMetallicTexture() == texture) { count++; }
			if (sd->materials[i]->getRoughnessTexture() == texture) { count++; }
			if (sd->materials[i]->getNormalTexture() == texture) { count++; }
			if (sd->materials[i]->getOclusionTexture() == texture) { count++; }
		}
		if (count <= 1)
		{
			for (int i = 0; i < sd->textures.size(); i++)
			{
				if (sd->textures[i] == texture)
				{
					sd->textures.erase(sd->textures.begin() + i);
					sd->textureData.erase(sd->textureData.begin() + i);
					break;
				}
			}
			tm->destroyTexture(texture);
		}
	}
}

void removeExistMaterial(Model* obj, SceneData* sd, MaterialManager* mm, TextureManager* tm, GraphicsDataMisc* gdm)
{
	if (sd != nullptr)
	{
		Materials* current = obj->getMaterial();
		int count = 0;
		if (current != nullptr)
		{
			for (int i = 0; i < sd->models.size(); i++)
			{
				if (sd->models[i]->getMaterial() == current)
				{
					count++;
				}
			}
			if (count <= 1)
			{
				removeExistTexture(current->getAlbedoTexture(), sd, tm, gdm);
				removeExistTexture(current->getMetallicTexture(), sd, tm, gdm);
				removeExistTexture(current->getRoughnessTexture(), sd, tm, gdm);
				removeExistTexture(current->getNormalTexture(), sd, tm, gdm);
				removeExistTexture(current->getOclusionTexture(), sd, tm, gdm);
				obj->setMaterial(nullptr);
				sd->materials.erase(std::remove(sd->materials.begin(), sd->materials.end(), current), sd->materials.end());
				if (gdm->str_default_material != current)
				{
					mm->destroyMaterial(current);
				}
			}
		}
	}
}

void Editor::deleteSceneObject(GObject * obj)
{
	if (obj == nullptr)
	{
		return;
	}
	const std::vector<GObject*> cchild = obj->getChilds();
	for (int c = 0; c < cchild.size(); c++)
	{
		deleteSceneObject(cchild[c]);
	}
	ShapeBuffer* sbCheck = nullptr;
	for (int i = 0; i < m_currentSceneData->models.size(); i++)
	{
		if ((GObject*)m_currentSceneData->models[i] == obj)
		{
			sbCheck = m_currentSceneData->models[i]->getShapeBuffer();
			Model* m = m_currentSceneData->models[i];
			if (m->getMaterial() != nullptr)
			{
				removeExistMaterial(m, m_currentSceneData, m_pc->materialManager, m_pc->textureManager, m_gdm);
			}
			m_currentSceneData->models.erase(m_currentSceneData->models.begin() + i);
			m_pc->modelManager->destroyModel(m);
			i = m_currentSceneData->models.size();
		}
	}
	if (sbCheck != nullptr)
	{
		bool found = false;
		int i = 0;
		for (; i < m_currentSceneData->models.size() && !found; i++)
		{
			if (m_currentSceneData->models[i]->getShapeBuffer() == sbCheck)
			{
				found = true;
			}
		}
		if (!found)
		{
			for (int j = 0; j < m_currentSceneData->buffer.size(); j++)
			{
				if (m_currentSceneData->buffer[j] == sbCheck)
				{
					m_currentSceneData->buffer.erase(m_currentSceneData->buffer.begin() + j);
					m_currentSceneData->bufferData.erase(m_currentSceneData->bufferData.begin() + j);
					break;
				}
			}
			m_pc->modelManager->destroyBuffer(sbCheck);
		}
	}
	SoundBuffer* soundbCheck = nullptr;
	for (int i = 0; i < m_currentSceneData->audio.size(); i++)
	{
		if ((GObject*)m_currentSceneData->audio[i] == obj)
		{
			soundbCheck = m_currentSceneData->audio[i]->getSoundBuffer();
			AudioSource* m = m_currentSceneData->audio[i];
			m_currentSceneData->audio.erase(m_currentSceneData->audio.begin() + i);
			m_pc->soundManager->releaseSource(m);
			i = m_currentSceneData->audio.size();
		}
	}
	if (soundbCheck != nullptr)
	{
		bool found = false;
		int i = 0;
		for (; i < m_currentSceneData->audio.size() && !found; i++)
		{
			if (m_currentSceneData->audio[i]->getSoundBuffer() == soundbCheck)
			{
				found = true;
			}
		}
		if (!found)
		{
			for (int j = 0; j < m_currentSceneData->sound.size(); j++)
			{
				if (m_currentSceneData->sound[j] == soundbCheck)
				{
					m_currentSceneData->sound.erase(m_currentSceneData->sound.begin() + j);
					m_currentSceneData->soundBufferData.erase(m_currentSceneData->soundBufferData.begin() + j);
					break;
				}
			}
			m_pc->soundManager->releaseBuffer(soundbCheck);
		}
	}
	for (int j = 0; j < m_currentSceneData->empty.size(); j++)
	{
		if ((GObject*)m_currentSceneData->empty[j] == obj)
		{
			delete m_currentSceneData->empty[j];
			m_currentSceneData->empty.erase(m_currentSceneData->empty.begin() + j);
			break;
		}
	}
	for (int j = 0; j < m_currentSceneData->dlight.size(); j++)
	{
		if ((GObject*)m_currentSceneData->dlight[j] == obj)
		{
			DirectionalLight* dl = m_currentSceneData->dlight[j];
			m_currentSceneData->dlight.erase(m_currentSceneData->dlight.begin() + j);
			m_pc->lightManager->destroyLight(dl);
			break;
		}
	}
	for (int j = 0; j < m_currentSceneData->plight.size(); j++)
	{
		if ((GObject*)m_currentSceneData->plight[j] == obj)
		{
			PointLight* dl = m_currentSceneData->plight[j];
			m_currentSceneData->plight.erase(m_currentSceneData->plight.begin() + j);
			m_pc->lightManager->destroyLight(dl);
			break;
		}
	}
	for (int j = 0; j < m_currentSceneData->slight.size(); j++)
	{
		if ((GObject*)m_currentSceneData->slight[j] == obj)
		{
			SpotLight* dl = m_currentSceneData->slight[j];
			m_currentSceneData->slight.erase(m_currentSceneData->slight.begin() + j);
			m_pc->lightManager->destroyLight(dl);
			break;
		}
	}
}

bool hasChild(GObject* parent, GObject* child)
{
	bool childFound = false;
	const std::vector<GObject*> & c = parent->getChilds();
	if (parent == child)
	{
		return true;
	}
	for (int i = 0; i < c.size(); i++)
	{
		if (hasChild(c[i], child))
		{
			childFound = true;
			break;
		}
	}
	return childFound;
}

std::string extractExtension(const std::string& fullPath)
{
	size_t dotPosition = fullPath.find_last_of('.');
	if (dotPosition == std::string::npos)
	{
		return "";
	}

	size_t slashPosition = fullPath.find_last_of("/\\");
	if (slashPosition != std::string::npos && slashPosition > dotPosition)
	{
		return "";
	}
	return fullPath.substr(dotPosition);
}

std::string Editor::dropTargetImage()
{
	std::string path = "";
	if (ImGui::BeginDragDropTarget())
	{ 
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_ITEM"))
		{			
			std::string data = (char *)payload->Data;
			data.resize(payload->DataSize/sizeof(char));
			struct stat sb;
			std::string ext = extractExtension(data);
			if (stat(data.c_str(), &sb) == 0 && (ext == ".png" || ext == ".jpeg"))
			{
				path = cropPath(data, m_baseProjectLocation);
			}		
		}
		ImGui::EndDragDropTarget();
	}
	return path;
}

void Editor::dtv(GObject * obj,int * id)
{
	(*id)++;
	int baseId = (*id);
	ImGui::PushID(obj);
	const std::vector<GObject*>& child = obj->getChilds();
	bool selected = m_selectedOBJ == obj;
	bool fdk = false;

	if (multiSelect)
	{
		fdk = multiSelected.find(baseId) != multiSelected.end();
		if (fdk)
		{
			if (multiSelected[baseId] == nullptr)
			{
				multiSelected[baseId] = obj;
			}
			if (m_selectedOBJ != obj)
			{
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
			}
			selected = true;
		}		
	}
	if (child.empty())
	{
		if (ImGui::TreeNodeEx(obj->getName()->c_str(), selected ? (ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_Leaf) : ImGuiTreeNodeFlags_Leaf))
		{
			ImGui::TreePop();
		}
	}
	else
	{
		if (ImGui::TreeNodeEx(obj->getName()->c_str(), selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None))
		{
			for (int i = 0; i < child.size(); i++)
			{
				dtv(child[i], id);
			}
			ImGui::TreePop();
		}
	}
	if (multiSelect && fdk && m_selectedOBJ != obj)
	{
		ImGui::PopStyleColor();
	}
	if (m_hasSwitch && m_switchTreeObj == baseId)
	{
		m_selectedOBJ = obj;
		m_hasSwitch = false;
	}
	if (ImGui::IsItemClicked() && !m_oneClickFrame)
	{
		m_oneClickFrame = true;
		m_selectedOBJ = obj;
		m_switchTreeObj = baseId;
		m_hasSwitch = false;
		if (multiSelect)
		{
			multiSelected[baseId] = m_selectedOBJ;
		}
	}	
	if (m_selectedOBJ == obj)
	{
		if (m_pc->inputManager->getKeyDown(GLFW_KEY_DELETE))
		{
			m_selectedOBJ = nullptr;
			m_switchTreeObj = -1;
			m_hasSwitch = false;
			deleteSceneObject(obj);
		}
		if (m_pc->inputManager->getKeyDown(GLFW_KEY_D) && m_pc->inputManager->getKey(GLFW_KEY_LEFT_CONTROL))
		{
			duplicateSceneObject(m_selectedOBJ);
		}
		if (m_pc->inputManager->getKeyDown(GLFW_KEY_LEFT_SHIFT) && !multiSelect)
		{
			multiSelected.clear();
			multiSelected[baseId] = obj;
			multiSelect = true;
		}
		if (m_pc->inputManager->getKeyDown(GLFW_KEY_DOWN))
		{
			m_switchTreeObj++;
			m_hasSwitch = true;
			if (multiSelect)
			{
				multiSelected[m_switchTreeObj] = nullptr;
			}
		}
		if (m_pc->inputManager->getKeyDown(GLFW_KEY_UP))
		{
			m_switchTreeObj--;
			m_hasSwitch = true;
			if (multiSelect)
			{
				multiSelected[m_switchTreeObj] = nullptr;
			}
		}
	}
	if (multiSelect && ImGui::IsItemHovered())
	{
		if (m_pc->inputManager->getKeyUp(GLFW_KEY_LEFT_SHIFT))
		{
			bool find = false;
			for (const auto& var : multiSelected)
			{
				if (var.second == obj)
				{
					find = true;
					break;
				}
			}
			if (!find)
			{
				for (auto& var : multiSelected)
				{
					if (var.second != nullptr)
					{
						if (!hasChild(var.second, obj))
						{
							var.second->setParent(obj);
						}
					}
				}
			}		
			multiSelect = false;
			multiSelected.clear();
		}		
	}
	ImGui::PopID();		
}

void Editor::drawTreeView()
{
	int id = 500;	

	for (int i = 0; i < m_currentSceneData->models.size();i++)
	{
		if (m_currentSceneData->models[i]->getParent() == nullptr)
		{
			dtv(m_currentSceneData->models[i], &id);
		}
	}	
	for (int i = 0; i < m_currentSceneData->dlight.size(); i++)
	{
		if (m_currentSceneData->dlight[i]->getParent() == nullptr)
		{
			dtv(m_currentSceneData->dlight[i], &id);
		}
	}
	for (int i = 0; i < m_currentSceneData->slight.size(); i++)
	{
		if (m_currentSceneData->slight[i]->getParent() == nullptr)
		{
			dtv(m_currentSceneData->slight[i], &id);
		}
	}	
	for (int i = 0; i < m_currentSceneData->plight.size(); i++)
	{
		if (m_currentSceneData->plight[i]->getParent() == nullptr)
		{
			dtv(m_currentSceneData->plight[i], &id);
		}
	}
	for (int i = 0; i < m_currentSceneData->audio.size(); i++)
	{
		if (m_currentSceneData->audio[i]->getParent() == nullptr)
		{
			dtv(m_currentSceneData->audio[i], &id);
		}
	}
	for (int i = 0; i < m_currentSceneData->empty.size(); i++)
	{
		if (m_currentSceneData->empty[i]->getParent() == nullptr)
		{
			dtv(m_currentSceneData->empty[i], &id);
		}			
	}
	if (multiSelect && m_pc->inputManager->getKeyUp(GLFW_KEY_LEFT_SHIFT))
	{
		if (m_pc->inputManager->getKey(GLFW_KEY_LEFT_CONTROL))
		{
			for (auto& var : multiSelected)
			{
				if (var.second != nullptr)
				{
					var.second->setParent(nullptr);
				}
			}
		}
		multiSelect = false;
		multiSelected.clear();
	}
	m_oneClickFrame = false;
}

void Editor::drawFolderContents(const std::string& basePath,std::string& path)
{
	int itemCount = 0;
	ImGuiStyle& style = ImGui::GetStyle();	
	ImVec4 col = style.Colors[ImGuiCol_Button];
	style.Colors[ImGuiCol_Button] = ImVec4(0, 0, 0, 0);
	std::vector<std::string> subPaths;
	std::string remainingPath = path.substr(basePath.length());
	std::istringstream iss(remainingPath);
	std::string subPath;
	while (std::getline(iss, subPath, '\\')) 
	{
		if (!subPath.empty())
		{
			subPaths.push_back(subPath);
		}
	}

	ImGui::BeginChild("##ButtonPath", ImVec2(0, 35), true);
	if (ImGui::Button("Asset"))
	{
		path = basePath;
	}
	for (size_t i = 0; i < subPaths.size(); ++i) 
	{
		ImGui::SameLine();
		if (ImGui::Button(subPaths[i].c_str())) 
		{
			std::string newPath = basePath;
			for (size_t j = 0; j <= i; ++j) 
			{
				newPath += + "\\" + subPaths[j];
			}
			if (path == newPath)
			{
				FolderDialog::openFolder(path);
			}
			path = newPath;
		}
	}
	ImGui::EndChild();
	int idIM = 10;
	std::string newpath = path;
	for (const auto& entry : fs::directory_iterator(path))
	{
		ImGui::BeginGroup();	
		ImGui::PushID(idIM++);
		if (entry.is_directory())
		{			
			if (ImGui::ImageButton(m_icon[0], ImVec2(m_iconSize, m_iconSize)))
			{
				newpath = entry.path().string();
			}			
		}
		else if (entry.path().extension() == ".png" || entry.path().extension() == ".jpeg")
		{
			if (ImGui::ImageButton(m_icon[4], ImVec2(m_iconSize, m_iconSize)))
			{
				if (m_pc->inputManager->getKey(GLFW_KEY_L))
				{
					m_pc->skybox->loadTextureSkybox(entry.path().string());
					m_currentSceneData->skyboxPath = cropPath(entry.path().string(), m_baseProjectLocation);
				}
			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{				
				ImGui::SetDragDropPayload("DND_DEMO_ITEM", entry.path().string().c_str(), entry.path().string().size() * sizeof(char));				
				ImGui::Text("Dragging image");
				ImGui::EndDragDropSource();
			}
		}
		else if (entry.path().extension() == ".scene")
		{
			if (ImGui::ImageButton(m_icon[3], ImVec2(m_iconSize, m_iconSize)))
			{
				globalSave();
				if (m_currentSceneData == nullptr)
				{
					m_currentSceneData = new SceneData();
					m_currentSceneData->path.has = false;
				}
				clearScene(m_currentSceneData);
				loadScene(entry.path().string(), m_currentSceneData);
			}
		}
		else if (entry.path().extension() == ".obj")
		{
			if (ImGui::ImageButton(m_icon[1], ImVec2(m_iconSize, m_iconSize)))
			{				
				addModelToScene(entry.path().string());
			}
		}
		else if (entry.path().extension() == ".fbx")
		{
			if (ImGui::ImageButton(m_icon[1], ImVec2(m_iconSize, m_iconSize)))
			{
				addModelToScene(entry.path().string(),true);
			}
		}
		else if (entry.path().extension() == ".wav")
		{
			if (ImGui::ImageButton(m_icon[2], ImVec2(m_iconSize, m_iconSize)))
			{
				addAudioToScene(entry.path().string());
			}
		}
		else
		{
			if (ImGui::ImageButton(m_icon[5], ImVec2(m_iconSize, m_iconSize)))
			{

			}
		}
		if (ImGui::IsItemHovered())
		{
			if (m_pc->inputManager->getKeyDown(GLFW_KEY_DELETE))
			{
				m_deleteOject = true;
				deletePath = entry.path().string();							
			}
		}
		ImGui::PopID();
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 100);
		ImGui::TextWrapped("%s", entry.path().filename().string().c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndGroup();
		if ((itemCount + 1) % m_columns != 0)
		{
			ImGui::SameLine();
		}

		itemCount++;
	}
	path = newpath;
	style.Colors[ImGuiCol_Button] = col;
}

Editor::Editor()
{
	s_instance = this;
}

Editor* Editor::instance()
{
	return s_instance;
}

void Editor::start()
{
	m_pc = Engine::getPtrClassAddr();
	m_pc->hud->addImgui(this);
	m_editorData = new EditorConfig();
	PathManager::getHomeDirectory();
	loadConfig(PathManager::getHomeDirectory()+"/config.json" , m_editorData);
	for (int i = 0; i < 13; i++)
	{
		std::string fn = "../Asset/Editor/" + std::to_string(i) + ".png";
		m_icon.push_back((ImTextureID)m_pc->textureManager->createTexture(fn.c_str())->getTextureID());
	}
	op = ImGuizmo::TRANSLATE;
}

void Editor::loadConfig(const std::string& filePath,EditorConfig* ec)
{
	std::ifstream file(filePath);
	if (!file.is_open()) 
	{
		saveConfig(filePath,ec);
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string jsonFile = buffer.str();
	file.close();
	JS::ParseContext context(jsonFile);
	context.parseTo(*ec);
}

void Editor::saveConfig(const std::string& filePath,EditorConfig* ec)
{
	std::string pretty_json = JS::serializeStruct(*ec);
	std::ofstream file(filePath);
	if (!file.is_open()) 
	{
		Debug::Error("Failed to save EditorConfig");
		return;
	}

	file << pretty_json;
	file.close();
}

void replaceChar(std::string& str, char from, char to) 
{
	size_t pos = str.find(from);
	while (pos != std::string::npos) 
	{
		str[pos] = to;
		pos = str.find(from, pos + 1);
	}
}


void Editor::clearCurrentProject()
{
	if(m_currentProjectData != nullptr)
	{ 
		delete m_currentProjectData;
		m_currentProjectData = nullptr;
	}
}

bool Editor::createDirectory(const std::string& path)
{
	m_tempFile = path;
	replaceChar(m_tempFile,'/','\\');
	return CreateDirectory(m_tempFile.c_str(), NULL);
}

void Editor::loadProject(const std::string& filePath, ProjectData* ec)
{
	std::ifstream file(filePath + "\\Setting.json");
	if (!file.is_open())
	{
		saveProject(filePath, ec);
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string jsonFile = buffer.str();
	file.close();
	JS::ParseContext context(jsonFile);
	context.parseTo(*ec);
	ec->assetPath = filePath;
	m_baseProjectLocation = filePath;
	PathManager::setHomeProjectPath(m_baseProjectLocation);
	m_currentProjectLocation = ec->assetPath;
}

void Editor::saveProject(const std::string& filePath, ProjectData* ec)
{
	std::string projectPath= filePath + "\\" + ec->projetName;
	m_baseProjectLocation = projectPath;
	createDirectory(projectPath);
	createDirectory(projectPath + "\\Scene");	
	createDirectory(projectPath + "\\Ressources");
	createDirectory(projectPath + "\\Ressources\\Model");
	createDirectory(projectPath + "\\Ressources\\Shader");
	std::string pretty_json = JS::serializeStruct(*ec);
	std::ofstream file(projectPath +"\\Setting.json");
	if (!file.is_open())
	{
		Debug::Error("Failed to save EditorConfig");
		return;
	}

	file << pretty_json;
	file.close();
}


std::string extractFileName(const std::string& fullPath) 
{
	size_t lastSlashPos = fullPath.find_last_of('\\');
	size_t lastDotPos = fullPath.find_last_of('.');
	if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastDotPos > lastSlashPos) 
	{
		return fullPath.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
	}
	else 
	{
		return "";
	}
}

std::string extractFolderPath(const std::string& fullPath) 
{
	size_t lastSlashPos = fullPath.find_last_of('\\');
	if (lastSlashPos != std::string::npos) 
	{
		return fullPath.substr(0, lastSlashPos);
	}
	else 
	{
		return "";
	}
}

void Editor::clearScene(SceneData* sd)
{
	m_selectedOBJ = nullptr;
	m_switchTreeObj = -1;
	for (int i = 0; i < sd->models.size(); i++)
	{
		m_pc->modelManager->destroyModel(sd->models[i]);
	}
	sd->models.clear();
	for (int i = 0; i < sd->buffer.size(); i++)
	{
		m_pc->modelManager->destroyBuffer(sd->buffer[i]);
	}
	sd->buffer.clear();
	for (int i = 0; i < sd->materials.size(); i++)
	{
		m_pc->materialManager->destroyMaterial(sd->materials[i]);
	}
	sd->materials.clear();
	for (int i = 0; i < sd->shader.size(); i++)
	{
		m_pc->graphiquePipelineManager->destroyPipeline(sd->shader[i]);
	}
	sd->shader.clear();
	for (int i = 0; i < sd->textures.size(); i++)
	{
		m_pc->textureManager->destroyTexture(sd->textures[i]);
	}
	sd->textures.clear();

	for (int i = 0; i < sd->dlight.size(); i++)
	{
		m_pc->lightManager->destroyLight(sd->dlight[i]);
	}
	sd->dlight.clear();
	for (int i = 0; i < sd->plight.size(); i++)
	{
		m_pc->lightManager->destroyLight(sd->plight[i]);
	}
	sd->plight.clear();
	for (int i = 0; i < sd->slight.size(); i++)
	{
		m_pc->lightManager->destroyLight(sd->slight[i]);
	}
	sd->slight.clear();
	for (int i = 0; i < sd->audio.size(); i++)
	{
		m_pc->soundManager->releaseSource(sd->audio[i]);
	}
	sd->audio.clear();
	for (int i = 0; i < sd->sound.size(); i++)
	{
		m_pc->soundManager->releaseBuffer(sd->sound[i]);
	}
	sd->sound.clear();
	for (int i = 0; i < sd->empty.size(); i++)
	{
		delete sd->empty[i];
	}		
	sd->empty.clear();
	m_pc->physicsEngine->DebugClearCollider();
	m_guizmo = false;
}

GraphiquePipeline* Editor::getPipelineByScene(const std::string& Name)
{
	for (int i = 0; i < m_currentSceneData->shaderData.size(); i++)
	{
		if (m_currentSceneData->shaderData[i].name == Name)
		{
			return m_currentSceneData->shader[i];
		}
	}
	Debug::Error("getPipelineByScene %s not found in this scene",Name.c_str());
	return nullptr;
}

std::vector<Materials*> Editor::getMaterialByScene(const std::string& Name)
{
	std::vector<Materials*> mat;
	GraphiquePipeline* gp = nullptr;
	for (int i = 0; i < m_currentSceneData->shaderData.size() && gp == nullptr; i++)
	{
		if (m_currentSceneData->shaderData[i].name == Name)
		{
			gp = m_currentSceneData->shader[i];
		}
	}
	if (gp == nullptr)
	{
		Debug::Error("getMaterialByScene %s not found in this scene", Name.c_str());
		return mat;
	}

	for (int i = 0; i < m_currentSceneData->materials.size(); i++)
	{
		if (m_currentSceneData->materials[i]->getPipeline() == gp)
		{
			mat.push_back(m_currentSceneData->materials[i]);
		}
	}
	return mat;
}

void Editor::loadSceneGame(const std::string& filePath, LoadSceneType type)
{			
	if (type == LoadSceneType::Override)
	{
		stopScene();
		clearScene(m_currentSceneData);
	}
	else
	{
		Debug::Warn("Additive not implemented");
	}
	m_tempFlyCamera = m_pc->cameraManager->getCurrentCamera();
	m_currentFovCS = m_tempFlyCamera->getFieldOfView();
	m_tempFlyCamera->setFieldOfView(0);
	m_gameChangeScene = true;
	loadScene(filePath, m_currentSceneData);
	playScene();	
}

void Editor::loadScene(const std::string& filePath, SceneData* sd)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		saveScene(filePath, sd);
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string jsonFile = buffer.str();
	file.close();
	JS::ParseContext context(jsonFile);
	context.parseTo(*sd);
	sd->name = extractFileName(filePath);
	sd->currentPath = cropPath(std::string(filePath), m_baseProjectLocation);
	m_currentProjectData->lastSceneOpen = sd->currentPath;
	sd->empty.clear();
	scriptObject.clear();
	collisionObj.clear();
	for (int i = 0; i < sd->emptyData.size(); i++)
	{
		Empty* e = new Empty();
		e->setName(sd->emptyData[i].name);
		e->setPosition(sd->emptyData[i].position);
		e->setRotation(sd->emptyData[i].rotation);
		e->setScale(sd->emptyData[i].scale);
		scriptObject[(GObject*)e] = sd->emptyData[i].scripts;
		collisionObj[(GObject*)e] = sd->emptyData[i].collisions;
		sd->empty.push_back(e);
	}
	sd->shader.clear();
	for (int i = 0; i < sd->shaderData.size(); i++)
	{		
		GraphiquePipeline* gp = m_pc->graphiquePipelineManager->createPipeline(m_baseProjectLocation+ sd->shaderData[i].frag, m_baseProjectLocation + sd->shaderData[i].vert, sd->shaderData[i].back, sd->shaderData[i].multiS, sd->shaderData[i].transparency, sd->shaderData[i].cullmode);
		sd->shader.push_back(gp);
	}
	sd->textures.clear();
	for (int i = 0; i < sd->textureData.size(); i++)
	{
		sd->textures.push_back(m_pc->textureManager->createTexture((m_baseProjectLocation+sd->textureData[i].path).c_str(), sd->textureData[i].filter));
	}
	sd->materials.clear();
	for (int i = 0; i < sd->materialData.size(); i++)
	{
		Materials* mat = m_pc->materialManager->createMaterial();
		mat->setColor(sd->materialData[i].albedo);
		mat->setOffset(sd->materialData[i].offset);
		mat->setTilling(sd->materialData[i].tilling);
		mat->setMetallic(sd->materialData[i].metallic);
		mat->setRoughness(sd->materialData[i].roughness);
		mat->setNormal(sd->materialData[i].normal);
		mat->setOclusion(sd->materialData[i].ao);
		mat->setEmission(sd->materialData[i].emit);
		mat->setDraw(sd->materialData[i].active);
		if (sd->materialData[i].albedoMap != -1)
		{
			mat->setAlbedoTexture(sd->textures[sd->materialData[i].albedoMap]);
		}
		if (sd->materialData[i].normalMap != -1)
		{
			mat->setNormalTexture(sd->textures[sd->materialData[i].normalMap]);
		}
		if (sd->materialData[i].metallicMap != -1)
		{
			mat->setMetallicTexture(sd->textures[sd->materialData[i].metallicMap]);
		}
		if (sd->materialData[i].RoughnessMap != -1)
		{
			mat->setRoughnessTexture(sd->textures[sd->materialData[i].RoughnessMap]);
		}
		if (sd->materialData[i].aoMap != -1)
		{
			mat->setOclusionTexture(sd->textures[sd->materialData[i].aoMap]);
		}
		if (sd->materialData[i].shader != -1)
		{
			mat->setPipeline(sd->shader[sd->materialData[i].shader]);
		}
		sd->materials.push_back(mat);
	}
	sd->buffer.clear();
	for (int i = 0; i < sd->bufferData.size(); i++)
	{		
		if (extractExtension(sd->bufferData[i].path) == ".fbx")
		{
			std::vector<ShapeBuffer*> sbb = m_pc->modelManager->allocateFBXBuffer((m_baseProjectLocation + sd->bufferData[i].path).c_str(), sd->bufferData[i].normalRecalculate, {});
			sd->buffer.push_back(sbb[0]);
			for (int i = 1; i < sbb.size(); i++)
			{
				m_pc->modelManager->destroyBuffer(sbb[i]);
				Debug::Warn("Editor not work with multiple model per file %s %d", (m_baseProjectLocation + sd->bufferData[i].path).c_str(), i);
			}
		}
		else
		{
			sd->buffer.push_back(m_pc->modelManager->allocateBuffer((m_baseProjectLocation + sd->bufferData[i].path).c_str(), sd->bufferData[i].normalRecalculate));
		}
	}
	sd->models.clear();
	for (int i = 0; i < sd->modelData.size(); i++)
	{
		if (sd->modelData[i].idBuffer >= 0)
		{			
			Model* m = m_pc->modelManager->createModel(sd->buffer[sd->modelData[i].idBuffer], sd->modelData[i].name);
			m->setPosition(sd->modelData[i].position);
			m->setRotation(sd->modelData[i].rotation);
			m->setScale(sd->modelData[i].scale);
			scriptObject[(GObject*)m] = sd->modelData[i].scripts;
			collisionObj[(GObject*)m] = sd->modelData[i].collisions;
			if (sd->modelData[i].idMaterial >= 0)
			{
				m->setMaterial(sd->materials[sd->modelData[i].idMaterial]);
			}
			sd->models.push_back(m);
		}
	}
	sd->dlight.clear();
	sd->plight.clear();
	sd->slight.clear();
	std::vector<Lights*> allLight;
	for (int i = 0; i < sd->lightData.size(); i++)
	{
		if (sd->lightData[i].status == 0)
		{
			DirectionalLight * dl = m_pc->lightManager->createDirectionalLight(glm::vec3(0, 0, 0), sd->lightData[i].color, sd->lightData[i].name);
			dl->setPosition(sd->lightData[i].position);
			dl->setRotation(sd->lightData[i].rotation);
			dl->setScale(sd->lightData[i].scale);
			dl->setRange(sd->lightData[i].range);
			dl->setSpotAngle(sd->lightData[i].spotAngle);
			dl->setshadow(sd->lightData[i].shadow);
			sd->dlight.push_back(dl);
			scriptObject[(GObject*)dl] = sd->lightData[i].scripts;
			collisionObj[(GObject*)dl] = sd->lightData[i].collisions;
			allLight.push_back(dl);
		}
		else if (sd->lightData[i].status == 1)
		{
			PointLight* pl = m_pc->lightManager->createPointLight(sd->lightData[i].position, sd->lightData[i].color, sd->lightData[i].name);
			pl->setPosition(sd->lightData[i].position);
			pl->setRotation(sd->lightData[i].rotation);
			pl->setScale(sd->lightData[i].scale);
			pl->setRange(sd->lightData[i].range);
			pl->setSpotAngle(sd->lightData[i].spotAngle);
			pl->setshadow(sd->lightData[i].shadow);
			sd->plight.push_back(pl);
			scriptObject[(GObject*)pl] = sd->lightData[i].scripts;
			collisionObj[(GObject*)pl] = sd->lightData[i].collisions;
			allLight.push_back(pl);
		}
		else if (sd->lightData[i].status == 2)
		{
			SpotLight* sl = m_pc->lightManager->createSpotLight(sd->lightData[i].position, sd->lightData[i].color,glm::vec3(0,0,0), sd->lightData[i].spotAngle, sd->lightData[i].name);
			sl->setPosition(sd->lightData[i].position);
			sl->setRotation(sd->lightData[i].rotation);
			sl->setScale(sd->lightData[i].scale);
			sl->setRange(sd->lightData[i].range);
			sl->setSpotAngle(sd->lightData[i].spotAngle);
			sl->setshadow(sd->lightData[i].shadow);
			sd->slight.push_back(sl);
			scriptObject[(GObject*)sl] = sd->lightData[i].scripts;
			collisionObj[(GObject*)sl] = sd->lightData[i].collisions;
			allLight.push_back(sl);
		}
	}
	sd->sound.clear();
	for (int i = 0; i < sd->soundBufferData.size(); i++)
	{
		sd->sound.push_back(m_pc->soundManager->createBuffer((m_baseProjectLocation+sd->soundBufferData[i].path).c_str()));
	}
	sd->audio.clear();
	for (int i = 0; i < sd->audioData.size(); i++)
	{
		if (sd->audioData[i].idBuffer >= 0)
		{
			AudioSource* as = m_pc->soundManager->createSource(sd->sound[sd->audioData[i].idBuffer], sd->audioData[i].name);
			as->setPosition(sd->audioData[i].position);
			as->setRotation(sd->audioData[i].rotation);
			as->setScale(sd->audioData[i].scale);
			as->setPitch(sd->audioData[i].pitch);
			as->setGain(sd->audioData[i].gain);
			as->setVelocity(sd->audioData[i].velocity);
			as->setLoop(sd->audioData[i].loop);
			as->setLoop(sd->audioData[i].rolloffFactor);
			as->setLoop(sd->audioData[i].maxDistance);
			as->setLoop(sd->audioData[i].refDistance);
			scriptObject[(GObject*)as] = sd->audioData[i].scripts;
			collisionObj[(GObject*)as] = sd->lightData[i].collisions;
			sd->audio.push_back(as);
		}
	}

	std::vector<GObject*> allGObject;
	for (int i = 0; i < sd->empty.size(); i++) { allGObject.push_back(sd->empty[i]); }
	for (int i = 0; i < sd->audio.size(); i++) { allGObject.push_back(sd->audio[i]); }
	for (int i = 0; i < sd->dlight.size(); i++) { allGObject.push_back(sd->dlight[i]); }
	for (int i = 0; i < sd->slight.size(); i++) { allGObject.push_back(sd->slight[i]); }
	for (int i = 0; i < sd->plight.size(); i++) { allGObject.push_back(sd->plight[i]); }
	for (int i = 0; i < sd->models.size(); i++) { allGObject.push_back(sd->models[i]); }

	for (int i = 0; i < sd->emptyData.size(); i++) { if (sd->emptyData[i].idParent != -1) { sd->empty[i]->setParent(allGObject[sd->emptyData[i].idParent]); } }
	for (int i = 0; i < sd->audioData.size(); i++) { if (sd->audioData[i].idParent != -1) { sd->audio[i]->setParent(allGObject[sd->audioData[i].idParent]); } }
	for (int i = 0; i < sd->lightData.size(); i++) { if (sd->lightData[i].idParent != -1) { allLight[i]->setParent(allGObject[sd->lightData[i].idParent]);}}
	for (int i = 0; i < sd->modelData.size(); i++) { if (sd->modelData[i].idParent != -1) { sd->models[i]->setParent(allGObject[sd->modelData[i].idParent]); } }

	Camera* cam = m_pc->cameraManager->getCurrentCamera();
	cam->setPosition(sd->freeCamPos);
	cam->setRotation(sd->freeCamRot);
	if (sd->skyboxPath.empty())
	{
		m_pc->skybox->clearTextureSkybox();
	}
	else
	{
		m_pc->skybox->loadTextureSkybox(m_baseProjectLocation + sd->skyboxPath);
	}
	PPSetting* setting = m_pc->postProcess->getPPSetting();
	setting->copy(sd->pp);
}

void Editor::globalSave()
{
	if (m_currentProjectData != nullptr)
	{
		saveProject(m_currentProjectData->projetPath, m_currentProjectData);
		if (m_currentSceneData != nullptr)
		{
			saveScene(m_baseProjectLocation+m_currentSceneData->currentPath, m_currentSceneData);
		}
	}
}

void Editor::addLightToScene(int type)
{
	if (m_currentSceneData != nullptr)
	{
		if (type == 0)
		{
			DirectionalLight* dl = m_pc->lightManager->createDirectionalLight(glm::vec3(-90.0f, 45.0f, 0.0f), glm::vec3(1, 1, 1));
			m_currentSceneData->dlight.push_back(dl);
			m_selectedOBJ = dl;
		}		
		else if(type == 1)
		{
			Camera* cam = m_pc->cameraManager->getCurrentCamera();
			PointLight* pl = m_pc->lightManager->createPointLight(cam->getPosition() + cam->transformDirectionAxeZ() * SPAWN_DISTANCE, glm::vec3(1, 1, 1));
			m_currentSceneData->plight.push_back(pl);			
			m_selectedOBJ = pl;
		}		
		else if (type == 2)
		{
			Camera* cam = m_pc->cameraManager->getCurrentCamera();
			SpotLight* sl = m_pc->lightManager->createSpotLight(cam->getPosition() + cam->transformDirectionAxeZ() * SPAWN_DISTANCE, glm::vec3(1, 1, 1),glm::vec3(0,0,0),0);
			m_currentSceneData->slight.push_back(sl);
			m_selectedOBJ = sl;
		}
	}
}

void Editor::addEmptyToScene()
{
	if (m_currentSceneData != nullptr)
	{
		Empty * e = new Empty();
		Camera* cam = m_pc->cameraManager->getCurrentCamera();
		e->setPosition(cam->getPosition() + cam->transformDirectionAxeZ() * SPAWN_DISTANCE);
		m_selectedOBJ = e;
		m_currentSceneData->empty.push_back(e);
	}
}

void Editor::playScene()
{
	for (const auto& var : collisionObj)
	{
		for (const auto& s : var.second)
		{
			CollisionShape* cs = nullptr;
			if (s.type == 0)//box
			{
				cs = new BoxShape(glm::vec3(s.data), s.mass);
			}
			else if (s.type == 1)
			{
				cs = new SphereShape(s.data.x, s.mass);
			}
			else if (s.type == 2)
			{
				cs = new CapsuleShape(s.data.x, s.data.y, s.mass);
			}
			if (cs != nullptr)
			{
				m_allCollisionLoaded.push_back(cs);
				CollisionWraper* cb = m_pc->physicsEngine->AllocateCollision(cs);
				m_allCollisionBodyLoaded.push_back(cb);
				cb->setPosition(s.position);
				cb->setEulerAngles(s.euler);
				m_pc->physicsEngine->AddCollision(cb);
			}
		}
	}
	for (const auto& var : scriptObject)
	{
		for (const auto& s : var.second)
		{
			Behaviour* b = FACTORY(Behaviour).create(s.pathName);
			var.first->addComponent(b);
			b->load(s.data);
			m_pc->behaviourManager->addBehaviour(b);
			m_allBehaviourLoaded.push_back(b);
		}
	}
	if (m_generatePathFindingNextPlay)
	{
		PathFindingScene* b = new PathFindingScene(m_currentSceneData->path.pathPosition, m_currentSceneData->path.zoneSize, m_currentSceneData->path.pointCount, m_baseProjectLocation + m_currentSceneData->path.pathFolder, m_currentSceneData->path.pathLiasonPercent);
		b->help();
		m_allBehaviourLoaded.push_back(b);
		m_pc->behaviourManager->addBehaviour(b);
		m_generatePathFindingNextPlay = false;
	}
	else if (m_currentSceneData->path.has)
	{
		PathFindingScene* b = new PathFindingScene(m_currentSceneData->path.pathPosition, m_currentSceneData->path.zoneSize, m_currentSceneData->path.pointCount, m_baseProjectLocation + m_currentSceneData->path.pathFolder, m_currentSceneData->path.pathLiasonPercent);
		b->loadFromFile();
		m_allBehaviourLoaded.push_back(b);
		//m_pc->behaviourManager->addBehaviour(b);
	}
}

void Editor::stopScene()
{
	for (int i = 0; i < m_allBehaviourLoaded.size(); i++)
	{
		m_pc->behaviourManager->removeBehaviour(m_allBehaviourLoaded[i], true);
		m_allBehaviourLoaded[i]->stop();
		delete m_allBehaviourLoaded[i];
	}
	m_allBehaviourLoaded.clear();
	for (int i = 0; i < m_allCollisionBodyLoaded.size(); i++)
	{
		m_pc->physicsEngine->ReleaseCollision(m_allCollisionBodyLoaded[i]);
	}
	m_allCollisionBodyLoaded.clear();
	for (int i = 0; i < m_allCollisionLoaded.size(); i++)
	{
		delete m_allCollisionLoaded[i];
	}
	m_allCollisionLoaded.clear();
}

void Editor::addMaterialToModel(Model* obj)
{
	if (m_currentSceneData != nullptr)
	{
		removeExistMaterial(obj, m_currentSceneData, m_pc->materialManager,m_pc->textureManager,m_gdm);
		Materials* mat = m_pc->materialManager->createMaterial();
		obj->setMaterial(mat);
		m_currentSceneData->materials.push_back(mat);
	}
}

void Editor::addDuplicateMaterialToModel(Model* obj)
{
	if (m_currentSceneData != nullptr)
	{
		Materials* mat = m_pc->materialManager->createMaterial();
		Materials* prevMat = obj->getMaterial();
		if (prevMat == nullptr)
		{
			return;
		}
		mat->setTilling(prevMat->getTilling());
		mat->setOffset(prevMat->getOffset());
		mat->setColor(prevMat->getColor());
		mat->setAlbedoTexture(prevMat->getAlbedoTexture());
		mat->setNormalTexture(prevMat->getNormalTexture());
		mat->setMetallicTexture(prevMat->getMetallicTexture());
		mat->setRoughnessTexture(prevMat->getRoughnessTexture());
		mat->setOclusionTexture(prevMat->getOclusionTexture());
		mat->setDraw(prevMat->getDraw());
		mat->setEmission(prevMat->getEmission());
		mat->setMetallic(prevMat->getMetallic());
		mat->setRoughness(prevMat->getRoughness());
		mat->setNormal(prevMat->getNormal());
		mat->setOclusion(prevMat->getOclusion());
		mat->setCastShadow(prevMat->getCastShadow());
		m_currentSceneData->materials.push_back(mat);
		removeExistMaterial(obj, m_currentSceneData, m_pc->materialManager, m_pc->textureManager, m_gdm);				
		obj->setMaterial(mat);		
	}
}

void Editor::addAudioToScene(const std::string& filePath)
{
	if (m_currentSceneData != nullptr)
	{
		SoundBuffer* sb = nullptr;
		for (int i = 0; i < m_currentSceneData->soundBufferData.size(); i++)
		{
			if ((m_baseProjectLocation+m_currentSceneData->soundBufferData[i].path) == filePath)
			{
				sb = m_currentSceneData->sound[i];
				i = m_currentSceneData->soundBufferData.size();
			}
		}
		if (sb == nullptr)
		{
			sb = m_pc->soundManager->createBuffer(filePath.c_str());
			m_currentSceneData->sound.push_back(sb);
			SoundBufferData sbd;
			std::string temp = filePath;
			sbd.path = cropPath(temp, m_baseProjectLocation).c_str();
			m_currentSceneData->soundBufferData.push_back(sbd);
		}
		AudioSource* au = m_pc->soundManager->createSource(sb, extractFileName(filePath));
		m_selectedOBJ = au;
		Camera* cam = m_pc->cameraManager->getCurrentCamera();
		au->setPosition(cam->getPosition() + cam->transformDirectionAxeZ() * SPAWN_DISTANCE);
		m_currentSceneData->audio.push_back(au);
	}
}


void Editor::addModelToScene(const std::string& filePath,bool fbx)
{
	if (m_currentSceneData != nullptr)
	{
		ShapeBuffer* sb = nullptr;
		bool normalR = false;
		for (int i = 0; i < m_currentSceneData->bufferData.size(); i++)
		{			
			if (m_baseProjectLocation+m_currentSceneData->bufferData[i].path == filePath)
			{
				sb = m_currentSceneData->buffer[i];				
				normalR = m_currentSceneData->bufferData[i].normalRecalculate;
				i = m_currentSceneData->bufferData.size();
			}
		}		
		if (sb == nullptr)
		{
			if (fbx)
			{
				std::vector<ShapeBuffer*> sl = m_pc->modelManager->allocateFBXBuffer(filePath.c_str(), normalR, {});
				sb = sl[0];
				for (int i = 1; i < sl.size(); i++)
				{
					m_pc->modelManager->destroyBuffer(sl[i]);
					Debug::Warn("Editor not work with multiple model per file %s %d", filePath.c_str(),i);
				}
			}
			else
			{
				sb = m_pc->modelManager->allocateBuffer(filePath.c_str(), normalR);
			}			
			m_currentSceneData->buffer.push_back(sb);
			BufferData bd;
			bd.normalRecalculate = normalR;
			std::string temp = filePath;
			bd.path = cropPath(temp,m_baseProjectLocation).c_str();
			m_currentSceneData->bufferData.push_back(bd);
		}
		Model* m = m_pc->modelManager->createModel(sb,extractFileName(filePath));
		m_selectedOBJ = m;
		Camera * cam = m_pc->cameraManager->getCurrentCamera();
		m->setPosition(cam->getPosition() + cam->transformDirectionAxeZ() * SPAWN_DISTANCE);
		m_currentSceneData->models.push_back(m);
	}
}

void Editor::saveScene(const std::string& filePath, SceneData* sd)
{
	Camera * cam = m_pc->cameraManager->getCurrentCamera();
	sd->freeCamPos = cam->getPosition();
	sd->freeCamRot = cam->getRotation();

	std::vector<GObject*> allGObject;
	for (int i = 0; i < sd->empty.size(); i++) { allGObject.push_back(sd->empty[i]); }
	for (int i = 0; i < sd->audio.size(); i++) { allGObject.push_back(sd->audio[i]); }
	for (int i = 0; i < sd->dlight.size(); i++) { allGObject.push_back(sd->dlight[i]); }
	for (int i = 0; i < sd->slight.size(); i++) { allGObject.push_back(sd->slight[i]); }
	for (int i = 0; i < sd->plight.size(); i++) { allGObject.push_back(sd->plight[i]); }
	for (int i = 0; i < sd->models.size(); i++) { allGObject.push_back(sd->models[i]); }

	sd->emptyData.clear();
	for (int i = 0; i < sd->empty.size(); i++)
	{
		EmptyData ed;
		ed.name = *sd->empty[i]->getName();
		ed.position = sd->empty[i]->getPosition();
		ed.rotation = sd->empty[i]->getRotation();
		ed.scale = sd->empty[i]->getScale();
		GObject* parent = sd->empty[i]->getParent();
		ed.scripts = scriptObject[(GObject*)sd->empty[i]];
		ed.collisions = collisionObj[(GObject*)sd->empty[i]];
		if (parent != nullptr)
		{
			for (int j = 0; j < allGObject.size(); j++)
			{
				if (allGObject[j] == parent)
				{
					ed.idParent = j;
					break;
				}
			}
		}
		sd->emptyData.push_back(ed);
	}
	sd->audioData.clear();
	for (int i = 0; i < sd->audio.size(); i++)
	{
		AudioSourceData asd;
		asd.name = *sd->audio[i]->getName();
		asd.position = sd->audio[i]->getPosition();
		asd.rotation = sd->audio[i]->getRotation();
		asd.scale = sd->audio[i]->getScale();
		asd.pitch = sd->audio[i]->getPitch();
		asd.gain = sd->audio[i]->getGain();
		asd.velocity = sd->audio[i]->getVelocity();
		asd.loop = sd->audio[i]->getLoop();
		asd.rolloffFactor = sd->audio[i]->getLoop();
		asd.maxDistance = sd->audio[i]->getLoop();
		asd.refDistance = sd->audio[i]->getLoop();
		asd.scripts = scriptObject[(GObject*)sd->audio[i]];
		asd.collisions = collisionObj[(GObject*)sd->audio[i]];
		for (int j = 0; j < sd->sound.size(); j++)
		{
			if (sd->sound[j] == sd->audio[i]->getSoundBuffer())
			{
				asd.idBuffer = j;
				j = sd->sound.size();
			}
		}
		GObject* parent = sd->audio[i]->getParent();
		if (parent != nullptr)
		{
			for (int j = 0; j < allGObject.size(); j++)
			{
				if (allGObject[j] == parent)
				{
					asd.idParent = j;
					break;
				}
			}
		}
		sd->audioData.push_back(asd);
	}

	sd->lightData.clear();
	for (int i = 0; i < sd->dlight.size(); i++)
	{
		LightData ld;
		ld.name = *sd->dlight[i]->getName();
		ld.position = sd->dlight[i]->getPosition();
		ld.rotation = sd->dlight[i]->getRotation();
		ld.scale = sd->dlight[i]->getScale();
		ld.color = sd->dlight[i]->getColors();
		ld.range = sd->dlight[i]->getRange();
		ld.spotAngle = sd->dlight[i]->getSpotAngle();
		ld.shadow = sd->dlight[i]->getshadow();
		ld.status = 0;
		ld.scripts = scriptObject[(GObject*)sd->dlight[i]];
		ld.collisions = collisionObj[(GObject*)sd->dlight[i]];
		GObject* parent = sd->dlight[i]->getParent();
		if (parent != nullptr)
		{
			for (int j = 0; j < allGObject.size(); j++)
			{
				if (allGObject[j] == parent)
				{
					ld.idParent = j;
					break;
				}
			}
		}
		sd->lightData.push_back(ld);
	}
	for (int i = 0; i < sd->plight.size(); i++)
	{
		LightData ld;
		ld.name = *sd->plight[i]->getName();
		ld.position = sd->plight[i]->getPosition();
		ld.rotation = sd->plight[i]->getRotation();
		ld.scale = sd->plight[i]->getScale();
		ld.color = sd->plight[i]->getColors();
		ld.range = sd->plight[i]->getRange();
		ld.spotAngle = sd->plight[i]->getSpotAngle();
		ld.shadow = sd->plight[i]->getshadow();
		ld.status = 1;
		ld.scripts = scriptObject[(GObject*)sd->plight[i]];
		ld.collisions = collisionObj[(GObject*)sd->plight[i]];
		GObject* parent = sd->plight[i]->getParent();
		if (parent != nullptr)
		{
			for (int j = 0; j < allGObject.size(); j++)
			{
				if (allGObject[j] == parent)
				{
					ld.idParent = j;
					break;
				}
			}
		}
		sd->lightData.push_back(ld);
	}
	for (int i = 0; i < sd->slight.size(); i++)
	{
		LightData ld;
		ld.name = *sd->slight[i]->getName();
		ld.position = sd->slight[i]->getPosition();
		ld.rotation = sd->slight[i]->getRotation();
		ld.scale = sd->slight[i]->getScale();
		ld.color = sd->slight[i]->getColors();
		ld.range = sd->slight[i]->getRange();
		ld.spotAngle = sd->slight[i]->getSpotAngle();
		ld.shadow = sd->slight[i]->getshadow();
		ld.status = 2;
		ld.scripts = scriptObject[(GObject*)sd->slight[i]];
		ld.collisions = collisionObj[(GObject*)sd->slight[i]];
		GObject* parent = sd->slight[i]->getParent();
		if (parent != nullptr)
		{
			for (int j = 0; j < allGObject.size(); j++)
			{
				if (allGObject[j] == parent)
				{
					ld.idParent = j;
					break;
				}
			}
		}
		sd->lightData.push_back(ld);
	}
	sd->materialData.clear();
	for (int i = 0; i < sd->materials.size(); i++)
	{		
		MaterialData md;
		md.albedo = sd->materials[i]->getColor();
		md.offset = sd->materials[i]->getOffset();
		md.tilling = sd->materials[i]->getTilling();
		md.metallic = sd->materials[i]->getMetallic();
		md.roughness = sd->materials[i]->getRoughness();
		md.normal = sd->materials[i]->getNormal();
		md.ao = sd->materials[i]->getOclusion();
		md.emit = sd->materials[i]->getEmission();
		md.active = sd->materials[i]->getDraw();		
		for (int j = 0; j < sd->textures.size(); j++)
		{			
			if (sd->textures[j] == sd->materials[i]->getAlbedoTexture())
			{
				md.albedoMap = j;
			}			
			if (sd->textures[j] == sd->materials[i]->getNormalTexture())
			{
				md.normalMap = j;
			}
			if (sd->textures[j] == sd->materials[i]->getMetallicTexture())
			{
				md.metallicMap = j;
			}
			if (sd->textures[j] == sd->materials[i]->getRoughnessTexture())
			{
				md.RoughnessMap = j;
			}
			if (sd->textures[j] == sd->materials[i]->getOclusionTexture())
			{
				md.aoMap = j;
			}
		}
		for (int j = 0; j < sd->shader.size(); j++)
		{
			if (sd->shader[j] == sd->materials[i]->getPipeline())
			{
				md.shader = j;
				j = sd->shader.size();
			}
		}
		sd->materialData.push_back(md);
	}
	sd->modelData.clear();
	for (int i = 0; i < sd->models.size(); i++)
	{
		ModelData md;
		md.name = *sd->models[i]->getName();
		md.position = sd->models[i]->getPosition();
		md.rotation = sd->models[i]->getRotation();
		md.scale = sd->models[i]->getScale();
		md.scripts = scriptObject[(GObject*)sd->models[i]];
		md.collisions = collisionObj[(GObject*)sd->models[i]];
		for (int j = 0; j < sd->buffer.size(); j++)
		{
			if (sd->buffer[j] == sd->models[i]->getShapeBuffer())
			{
				md.idBuffer = j;
				j = sd->buffer.size();
			}			
		}
		for (int j = 0; j < sd->materials.size(); j++)
		{
			if (sd->materials[j] == sd->models[i]->getMaterial())
			{
				md.idMaterial = j;
				j = sd->materials.size();
			}
		}
		GObject* parent = sd->models[i]->getParent();
		if (parent != nullptr)
		{
			for (int j = 0; j < allGObject.size(); j++)
			{
				if (allGObject[j] == parent)
				{
					md.idParent = j;
					break;
				}
			}
		}
		sd->modelData.push_back(md);
	}
	std::string pretty_json = JS::serializeStruct(*sd);
	std::ofstream file(filePath);
	if (!file.is_open())
	{
		Debug::Error("Failed to save Scene at %s", filePath.c_str());
		return;
	}

	file << pretty_json;
	file.close();
}

void Editor::fixedUpdate()
{

}

void Editor::update()
{
	if (m_pc->inputManager->getKeyDown(GLFW_KEY_INSERT))
	{
		m_hide = !m_hide;
	}
	if (!m_hide)
	{
		m_colRGB = HSVtoRGB(fmod(m_pc->time->getTime() * 36.0f, 360.0f), 1.0f, 1.0f);
	}
	if(m_gameChangeScene)
	{
		m_gameChangeScene = false;
		m_tempFlyCamera->setFieldOfView(m_currentFovCS);
	}
}

void Editor::stop()
{
	globalSave();
	saveConfig(PathManager::getHomeDirectory() + "/config.json", m_editorData);
	delete m_editorData;
	if (m_currentProjectData != nullptr)
	{
		delete m_currentProjectData;
	}
	if (m_currentSceneData != nullptr)
	{
		delete m_currentSceneData;
	}
	m_pc->hud->removeImgui(this);
}

void Editor::onGUI()
{

}

void Editor::init(GraphicsDataMisc* gdm)
{
	m_gdm = gdm;
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.64f);		
}


void Editor::render(GraphicsDataMisc* gdm)
{
	if (m_hide)
	{
		return;
	}
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 wbg = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);
	style.Colors[ImGuiCol_WindowBg] = wbg;	
	ImVec4 colbbg = style.Colors[ImGuiCol_Button];
	int mainWindowPosX, mainWindowPosY;
	glfwGetWindowPos(gdm->str_window, &mainWindowPosX, &mainWindowPosY);

	if (m_pc->inputManager->getKeyDown(GLFW_KEY_N) && m_pc->inputManager->getKey(GLFW_KEY_LEFT_CONTROL))
	{
		m_newProjectModal = true;
	}
	if (m_pc->inputManager->getKeyDown(GLFW_KEY_O) && m_pc->inputManager->getKey(GLFW_KEY_LEFT_CONTROL))
	{
		m_openProjectModal = true;
	}
	if (m_pc->inputManager->getKeyDown(GLFW_KEY_S) && m_pc->inputManager->getKey(GLFW_KEY_LEFT_CONTROL) && m_playMode == 0)
	{
		globalSave();		
	}
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctrl+N"))
			{
				m_newProjectModal = true;				
			}
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
				m_openProjectModal = true;
			}
			if (m_currentProjectData != nullptr)
			{
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					globalSave();
				}
			}
			ImGui::EndMenu();
		}
		if (m_currentProjectData != nullptr)
		{		
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Scene"))
				{
					m_createObject = true;
					objType = ObjectType::SceneOBJ;
				}
				if (ImGui::MenuItem("Directional Light"))
				{
					addLightToScene(0);
				}
				if (ImGui::MenuItem("Point Light"))
				{
					addLightToScene(1);
				}
				if (ImGui::MenuItem("Spot Light"))
				{
					addLightToScene(2);
				}
				if (ImGui::MenuItem("Empty"))
				{
					addEmptyToScene();
				}
				if (ImGui::MenuItem("PathFinding"))
				{							
					if (m_currentSceneData != nullptr)
					{
						m_openPathFinding = true;
					}
				}
				if (ImGui::MenuItem("Skybox Clear"))
				{
					m_pc->skybox->clearTextureSkybox();
					if (m_currentSceneData != nullptr)
					{						
						m_currentSceneData->skyboxPath = "";
					}
				}
				if (ImGui::MenuItem("Shader"))
				{
					if (m_currentSceneData != nullptr)
					{
						m_openPathShader = true;
					}
				}
				ImGui::EndMenu();
			}
		}
				
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Game"))
			{
				m_editorData->gameMode = !m_editorData->gameMode;
				ImGui::SetWindowFocus("Game Mode");
			}
			if (ImGui::MenuItem("Inspector"))
			{
				m_editorData->inspector = true;
				ImGui::SetWindowFocus("Inspector");
			}
			if (ImGui::MenuItem("Hiearchy"))
			{
				m_editorData->hiearchy = true;
				ImGui::SetWindowFocus("Hiearchy");
			}
			if (ImGui::MenuItem("Project"))
			{
				m_editorData->project = true;
				ImGui::SetWindowFocus("Project");
			}
			if (ImGui::MenuItem("PostProcess"))
			{
				m_editorData->postProcess = true;
				ImGui::SetWindowFocus("PostProcess");
			}
			ImGui::EndMenu();
		}		
		if (m_currentProjectData != nullptr)
		{
			ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(m_currentProjectData->projetName).x-15);
			ImGui::TextColored(m_colRGB,m_currentProjectData->projetName);
		}
		ImGui::EndMainMenuBar();
	}

	if (m_openPathFinding)
	{
		ImGui::OpenPopup("PathFinding");
		m_currentSceneData->path.pathPosition = glm::vec3(0,0,0);
		m_currentSceneData->path.zoneSize = glm::vec3(10, 10, 10);
		m_currentSceneData->path.pointCount = glm::vec3(10, 10, 10);
		m_currentSceneData->path.pathLiasonPercent = 0.1f;		
		m_currentSceneData->path.has = false;
		std::string computeName = cropPath(m_currentProjectLocation + "\\" + m_currentSceneData->name + ".path", m_baseProjectLocation);
		strcpy(m_pathFindingName, computeName.c_str());
		m_openPathFinding = false;
	}

	if (m_openPathShader)
	{
		ImGui::OpenPopup("ShaderPath");	
		m_pathOpenProject[0] = '\0';
		m_objectName[0] = '\0';
		m_pathFindingName[0] = '\0';
		currentWriteSD.back = false;
		currentWriteSD.multiS = true;
		currentWriteSD.transparency = false;
		currentWriteSD.cullmode = 1;		
		m_openPathShader = false;
	}
	if (ImGui::BeginPopupModal("ShaderPath") && m_currentSceneData != nullptr)
	{
		ImGui::InputText("Name ##ShaderPath", m_pathOpenProject, IM_ARRAYSIZE(m_pathOpenProject));

		ImGui::InputText("Fragment Shader##ShaderPath", m_objectName, IM_ARRAYSIZE(m_objectName));
		ImGui::SameLine();
		if (ImGui::Button("...##Fragment"))
		{
			std::string path = FolderDialog::openFileDialog();
			strncpy(m_objectName, path.c_str(), sizeof(m_objectName) - 1);
			m_objectName[sizeof(m_objectName) - 1] = '\0';
		}

		ImGui::InputText("Vertex Shader##ShaderPath", m_pathFindingName, IM_ARRAYSIZE(m_pathFindingName));
		ImGui::SameLine();
		if (ImGui::Button("...##Vertex"))
		{
			std::string path = FolderDialog::openFileDialog();
			strncpy(m_pathFindingName, path.c_str(), sizeof(m_pathFindingName) - 1);
			m_pathFindingName[sizeof(m_pathFindingName) - 1] = '\0';
		}

		ImGui::Checkbox("Back##shader",&currentWriteSD.back);
		ImGui::Checkbox("MultiS##shader", &currentWriteSD.multiS);
		ImGui::Checkbox("Transparency##shader", &currentWriteSD.transparency);
		ImGui::DragInt("Cullmode[0,1,2]", &currentWriteSD.cullmode);

		if (ImGui::Button("Apply"))
		{
			ShaderData nsd;
			nsd.name = m_pathOpenProject;
			nsd.frag = m_objectName;
			nsd.vert = m_pathFindingName;

			nsd.frag = cropPath(nsd.frag, m_baseProjectLocation);
			nsd.vert = cropPath(nsd.vert, m_baseProjectLocation);

			nsd.back = currentWriteSD.back;
			nsd.multiS = currentWriteSD.multiS;
			nsd.transparency = currentWriteSD.transparency;
			nsd.cullmode = currentWriteSD.cullmode;
			m_currentSceneData->shaderData.push_back(nsd);
			GraphiquePipeline* gp = m_pc->graphiquePipelineManager->createPipeline(m_baseProjectLocation + nsd.frag, m_baseProjectLocation + nsd.vert, nsd.back, nsd.multiS, nsd.transparency, nsd.cullmode);
			m_currentSceneData->shader.push_back(gp);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("PathFinding") && m_currentSceneData != nullptr)
	{
		ImGui::InputText("PathFinding##inputFieldPathOpen", m_pathFindingName, IM_ARRAYSIZE(m_pathFindingName));
		ImGui::SameLine();
		if (ImGui::Button("..."))
		{
			std::string path = FolderDialog::openDialog();
			strncpy(m_pathFindingName, path.c_str(), sizeof(m_pathFindingName) - 1);
			m_pathFindingName[sizeof(m_pathFindingName) - 1] = '\0';
		}
		ImGui::DragFloat3("Position", &m_currentSceneData->path.pathPosition[0]);
		ImGui::DragFloat3("Zone", &m_currentSceneData->path.zoneSize[0]);
		ImGui::DragFloat3("Point", &m_currentSceneData->path.pointCount[0]);
		ImGui::DragFloat("LiasonPercent", &m_currentSceneData->path.pathLiasonPercent);
		if (ImGui::Button("Apply"))
		{
			m_generatePathFindingNextPlay = true;
			m_currentSceneData->path.pathFolder = m_pathFindingName;
			m_currentSceneData->path.pathFolder = cropPath(m_currentSceneData->path.pathFolder, m_baseProjectLocation);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (m_newProjectModal)
	{
		ImGui::OpenPopup("New Project");
		clearCurrentProject();
		m_currentProjectData = new ProjectData();
		m_newProjectModal = false;
	}

	if (m_openProjectModal)
	{
		ImGui::OpenPopup("Open Project");
		clearCurrentProject();
		m_pathOpenProject[0] = '\0';
		if (m_editorData->lastProjectpathOpen.size() != 0)
		{
			strncpy(m_pathOpenProject, m_editorData->lastProjectpathOpen.c_str(), sizeof(m_pathOpenProject) - 1);
			m_pathOpenProject[sizeof(m_pathOpenProject) - 1] = '\0';
		}
		m_openProjectModal = false;
	}

	if (m_deleteOject)
	{
		ImGui::OpenPopup("Delete");
		m_deleteOject = false;
	}
	if (ImGui::BeginPopupModal("Delete", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure ?");
		if (ImGui::Button("Yes", ImVec2(100, 0)) || m_pc->inputManager->getKeyDown(GLFW_KEY_ENTER))
		{
			if (std::remove(deletePath.c_str()) != 0)
			{
				Debug::Error("Error deleting file: %s", deletePath.c_str());
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(100, 0)) || m_pc->inputManager->getKeyDown(GLFW_KEY_ESCAPE))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
		return;
	}

	if (ImGui::BeginPopupModal("Open Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Path##inputFieldPathOpen", m_pathOpenProject, IM_ARRAYSIZE(m_pathOpenProject));
		ImGui::SameLine();
		if (ImGui::Button("..."))
		{
			std::string path = FolderDialog::openDialog();
			strncpy(m_pathOpenProject, path.c_str(), sizeof(m_pathOpenProject) - 1);
			m_pathOpenProject[sizeof(m_pathOpenProject) - 1] = '\0';
		}

		if (ImGui::Button("Load", ImVec2(100, 0)) || m_pc->inputManager->getKeyDown(GLFW_KEY_ENTER))
		{
			if (m_pathOpenProject[0] != '\0')
			{
				struct stat sb;
				m_editorData->lastProjectpathOpen = m_pathOpenProject;
				if (stat(m_pathOpenProject, &sb) == 0)
				{
					globalSave();
					if (m_currentSceneData != nullptr)
					{
						clearScene(m_currentSceneData);
						m_currentSceneData = nullptr;
						delete m_currentSceneData;
					}
					m_currentProjectData = new ProjectData();
					loadProject(m_pathOpenProject, m_currentProjectData);
					if (!m_currentProjectData->lastSceneOpen.empty())
					{
						SceneData* sd = new SceneData();
						m_currentSceneData = sd;
						loadScene(m_baseProjectLocation+m_currentProjectData->lastSceneOpen, m_currentSceneData);
					}
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(100, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
		return;
	}

	if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{		
		ImGui::InputText("Name##inputFieldName", m_currentProjectData->projetName, IM_ARRAYSIZE(m_currentProjectData->projetName));
		ImGui::InputText("Path##inputFieldPath", m_currentProjectData->projetPath, IM_ARRAYSIZE(m_currentProjectData->projetPath));
		ImGui::SameLine();
		if (ImGui::Button("..."))
		{
			std::string path = FolderDialog::openDialog();
			strncpy(m_currentProjectData->projetPath, path.c_str(), sizeof(m_currentProjectData->projetPath) - 1);
			m_currentProjectData->projetPath[sizeof(m_currentProjectData->projetPath) - 1] = '\0';
		}

		if (ImGui::Button("Create", ImVec2(100, 0)) || m_pc->inputManager->getKeyDown(GLFW_KEY_ENTER))
		{
			if (m_currentProjectData->projetName[0] != '\0' && m_currentProjectData->projetPath[0] != '\0')
			{
				struct stat sb;
				if (stat(m_currentProjectData->projetPath, &sb) == 0)
				{
					globalSave();
					if (m_currentSceneData != nullptr)
					{
						clearScene(m_currentSceneData);
						m_currentSceneData = nullptr;
						delete m_currentSceneData;
					}
					m_currentProjectData->ressourcePath = "";
					m_currentProjectData->ressourcePath += m_currentProjectData->projetPath;
					m_currentProjectData->ressourcePath += "\\";
					m_currentProjectData->ressourcePath += m_currentProjectData->projetName; 
					m_currentProjectData->assetPath = "";
					m_currentProjectData->assetPath += m_currentProjectData->ressourcePath;
					m_currentProjectData->ressourcePath += "\\Ressources";
					m_currentProjectLocation = m_currentProjectData->assetPath;
					m_currentProjectData->lastSceneOpen = "\\Scene\\Main.scene";
					saveProject(m_currentProjectData->projetPath, m_currentProjectData);
					if (!copyAndRenameDirectory("..\\Asset\\Model", m_currentProjectData->assetPath + "\\Ressources\\Model\\BaseModel"))
					{
						Debug::Error("Failed to copy Base Model");
					}
					SceneData* sd = new SceneData();
					m_currentSceneData = sd;
					m_currentSceneData->path.has = false;
					saveScene(m_baseProjectLocation +m_currentProjectData->lastSceneOpen, m_currentSceneData);
					m_currentSceneData->currentPath = m_currentProjectData->lastSceneOpen;
					m_currentSceneData->name = "Main";
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(100, 0)))
		{
			delete m_currentProjectData;
			m_currentProjectData = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
		return;
	}

	if (m_createObject)
	{
		ImGui::OpenPopup("Create Object");
		m_objectName[0] = '\0';
		m_createObject = false;
	}
	if (ImGui::BeginPopupModal("Create Object", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{		
		ImGui::InputText("Name##inputFieldObject", m_objectName, IM_ARRAYSIZE(m_objectName));
		if (ImGui::Button("Create", ImVec2(100, 0)) || m_pc->inputManager->getKeyDown(GLFW_KEY_ENTER))
		{
			if (m_objectName != '\0')
			{
				struct stat sb;
				if (stat(m_currentProjectLocation.c_str(), &sb) == 0)
				{
					if (objType == ObjectType::SceneOBJ)
					{
						if (m_currentSceneData != nullptr)
						{							
							saveScene(m_baseProjectLocation +m_currentSceneData->currentPath, m_currentSceneData);
							clearScene(m_currentSceneData);
							delete m_currentSceneData;
							m_currentSceneData = nullptr;
						}
						SceneData* sd = new SceneData();
						m_currentSceneData = sd;
						m_currentSceneData->path.has = false;
						sd->name = m_objectName;
						sd->currentPath = cropPath(m_currentProjectLocation + "\\" + m_objectName + ".scene", m_baseProjectLocation);
						m_currentProjectData->lastSceneOpen = sd->currentPath;
						sd->dlight.push_back(m_pc->lightManager->createDirectionalLight(glm::vec3(-45,45,0),glm::vec3(1,1,1)));
						globalSave();
					}
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(100, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
		return;
	}
	
	if (m_editorData->gameMode)
	{
		ImGui::SetNextWindowSize(ImVec2(250,100));

		ImGui::SetNextWindowPos(ImVec2(mainWindowPosX+(m_pc->settingManager->getWindowWidth() / 2 - 125), 25+ mainWindowPosY));
		ImGui::Begin("Game Mode", &m_editorData->gameMode, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
		style.Colors[ImGuiCol_Button] = ImVec4(0, 0, 0, 0);
		int popCol = 0;
		if (m_playMode == 1) 
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.7f, 0.0f, 1.0f));
			popCol++;
		}
		if (ImGui::ImageButton(m_icon[6], ImVec2(m_iconModeSize, m_iconModeSize)) && m_playMode != 1 && m_currentSceneData != nullptr)
		{
			if (m_playMode == 2)
			{
				for (int i = 0; i < m_allBehaviourLoaded.size(); i++)
				{
					m_pc->behaviourManager->addBehaviourWithoutStart(m_allBehaviourLoaded[i]);
				}
			}
			else
			{				
				globalSave();
				playScene();
			}
			m_playMode = 1;
		}
		if (popCol == 1) {
			ImGui::PopStyleColor(); 
			popCol--;
		}
		ImGui::SameLine();
		if (m_playMode == 2) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.0f, 0.7f, 1.0f));
			popCol++;
		}
		if (ImGui::ImageButton(m_icon[7], ImVec2(m_iconModeSize, m_iconModeSize)) && m_playMode != 2)
		{
			m_playMode = 2;
			for (int i = 0; i < m_allBehaviourLoaded.size(); i++)
			{
				m_pc->behaviourManager->removeBehaviour(m_allBehaviourLoaded[i], true);
			}
		}
		if (popCol == 1) 
		{
			ImGui::PopStyleColor();
			popCol--;
		}
		if (m_playMode == 0) 
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.0f, 0.0f, 1.0f));
			popCol++;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(m_icon[8], ImVec2(m_iconModeSize, m_iconModeSize)) && m_playMode != 0)
		{
			m_playMode = 0;
			stopScene();
			m_allCollisionLoaded.clear();
			clearScene(m_currentSceneData);
			loadScene(m_baseProjectLocation +m_currentProjectData->lastSceneOpen, m_currentSceneData);
			m_pc->inputManager->HideMouse(false);
		}		
		if (popCol == 1) 
		{
			ImGui::PopStyleColor();
			popCol--;
		}
		style.Colors[ImGuiCol_Button] = colbbg;
		ImGui::SameLine();
		ImGui::End();
	}

	if (m_editorData->inspector)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Inspector", &m_editorData->inspector);
		if (m_selectedOBJ != nullptr && m_currentSceneData != nullptr)
		{
			m_selectedOBJ->onGUI();
			if (Model* model = dynamic_cast<Model*>(m_selectedOBJ))
			{
				if (ImGui::Button("Add Material"))
				{
					addMaterialToModel(model);
				}
				ImGui::SameLine();
				if (ImGui::Button("Add(Duplicate) Material"))
				{
					addDuplicateMaterialToModel(model);
				}
				ImGui::SameLine();
				if (ImGui::Button("Clear Matertial"))
				{
					removeExistMaterial(model, m_currentSceneData, m_pc->materialManager, m_pc->textureManager, m_gdm);
					model->setMaterial(gdm->str_default_material);
				}
				if (ImGui::Button("Next"))
				{
					removeExistMaterial(model, m_currentSceneData, m_pc->materialManager, m_pc->textureManager, m_gdm);
					if (m_currentSceneData->materials.size() > 0)
					{
						model->setMaterial(m_currentSceneData->materials[matCN % m_currentSceneData->materials.size()]);
						matCN++;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Prev"))
				{
					removeExistMaterial(model, m_currentSceneData, m_pc->materialManager, m_pc->textureManager, m_gdm);
					if (m_currentSceneData->materials.size() > 0)
					{
						model->setMaterial(m_currentSceneData->materials[matCN % m_currentSceneData->materials.size()]);
						matCN--;
						if (matCN < 0)
						{
							matCN = m_currentSceneData->materials.size();
						}
					}
				}
				Materials* mat = model->getMaterial();
				if (mat != nullptr)
				{
					ImGui::SameLine();
					int shaderID = -1;
					for (int i = 0; i < m_currentSceneData->shader.size() && shaderID == -1; i++)
					{
						if (mat->getPipeline() == m_currentSceneData->shader[i])
						{
							shaderID = i;
						}
					}
					
					if (ImGui::BeginCombo("##ShaderCombo", shaderID == -1 ? "Default" : m_currentSceneData->shaderData[shaderID].name.c_str()))
					{
						int size = m_currentSceneData->shader.size();
						for (int n = -1; n < size; n++)
						{
							const bool is_selected = (shaderID == n);
							if (ImGui::Selectable(((n == -1 ? "Default" : m_currentSceneData->shaderData[n].name)+"##Shader").c_str(), is_selected))
							{
								if (n == -1)
								{
									mat->setPipeline(gdm->str_default_pipeline);
								}
								else
								{
									mat->setPipeline(m_currentSceneData->shader[n]);
								}
							}

							if (is_selected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}

						ImGui::EndCombo();
					}				
					bool filter = !m_pc->inputManager->getKey(GLFW_KEY_N);
					ImGui::Text("Base       ");
					ImGui::SameLine();
					ImGui::Image((ImTextureID)mat->getAlbedoTexture()->getTextureID(),ImVec2(80, 80));
					if (ImGui::IsItemClicked(0))
					{
						for (int i = 0; i < m_currentSceneData->textures.size(); i++)
						{
							if (m_currentSceneData->textures[i] == mat->getAlbedoTexture())
							{								
								m_currentProjectLocation = m_baseProjectLocation + removeFilename(m_currentSceneData->textureData[i].path);
							}
						}
					}
					std::string path = dropTargetImage();
					if (!path.empty())
					{
						removeExistTexture(mat->getAlbedoTexture(), m_currentSceneData, m_pc->textureManager, gdm);
						bool found = false;
						Textures* t = nullptr;
						
						for (int i = 0; i < m_currentSceneData->textureData.size(); i++)
						{
							if (m_currentSceneData->textureData[i].path == path && m_currentSceneData->textureData[i].filter == filter)
							{
								t = m_currentSceneData->textures[i];
								found = true;
							}
						}
						if (!found)
						{
							t = m_pc->textureManager->createTexture((m_baseProjectLocation+path).c_str(), filter);
						}
						mat->setAlbedoTexture(t);
						if (mat != m_gdm->str_default_material && !found)
						{							
							m_currentSceneData->textures.push_back(t);
							TextureData td;
							td.path = path;
							td.filter = t->getFilter();
							m_currentSceneData->textureData.push_back(td);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear##Base")) { removeExistTexture(mat->getAlbedoTexture(), m_currentSceneData, m_pc->textureManager, gdm); mat->setAlbedoTexture(nullptr); }

					ImGui::Text("Metallic   ");
					ImGui::SameLine();
					ImGui::Image((ImTextureID)mat->getMetallicTexture()->getTextureID(), ImVec2(80, 80));
					if (ImGui::IsItemClicked(0))
					{
						for (int i = 0; i < m_currentSceneData->textures.size(); i++)
						{
							if (m_currentSceneData->textures[i] == mat->getMetallicTexture())
							{
								m_currentProjectLocation = m_baseProjectLocation + removeFilename(m_currentSceneData->textureData[i].path);
							}
						}
					}
					path = dropTargetImage();
					if (!path.empty())
					{
						removeExistTexture(mat->getMetallicTexture(), m_currentSceneData, m_pc->textureManager, gdm);
						bool found = false;
						Textures* t = nullptr;
						for (int i = 0; i < m_currentSceneData->textureData.size(); i++)
						{
							if (m_currentSceneData->textureData[i].path == path && m_currentSceneData->textureData[i].filter == filter)
							{
								t = m_currentSceneData->textures[i];
								found = true;
							}
						}
						if (!found)
						{
							t = m_pc->textureManager->createTexture((m_baseProjectLocation+path).c_str(), filter);
						}
						mat->setMetallicTexture(t);
						if (mat != m_gdm->str_default_material && !found)
						{							
							m_currentSceneData->textures.push_back(t);
							TextureData td;
							td.path = path;
							td.filter = t->getFilter();
							m_currentSceneData->textureData.push_back(td);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear##Metallic")) { removeExistTexture(mat->getMetallicTexture(), m_currentSceneData, m_pc->textureManager, gdm); mat->setMetallicTexture(nullptr); }

					ImGui::Text("Roughness  ");
					ImGui::SameLine();
					ImGui::Image((ImTextureID)mat->getRoughnessTexture()->getTextureID(), ImVec2(80, 80));
					if (ImGui::IsItemClicked(0))
					{
						for (int i = 0; i < m_currentSceneData->textures.size(); i++)
						{
							if (m_currentSceneData->textures[i] == mat->getRoughnessTexture())
							{
								m_currentProjectLocation = m_baseProjectLocation + removeFilename(m_currentSceneData->textureData[i].path);
							}
						}
					}
					path = dropTargetImage();
					if (!path.empty())
					{
						removeExistTexture(mat->getRoughnessTexture(), m_currentSceneData, m_pc->textureManager, gdm);
						bool found = false;
						Textures* t = nullptr;
						for (int i = 0; i < m_currentSceneData->textureData.size(); i++)
						{
							if (m_currentSceneData->textureData[i].path == path && m_currentSceneData->textureData[i].filter == filter)
							{
								t = m_currentSceneData->textures[i];
								found = true;
							}
						}
						if (!found)
						{
							t = m_pc->textureManager->createTexture((m_baseProjectLocation+path).c_str(), filter);
						}
						mat->setRoughnessTexture(t);
						
						if (mat != m_gdm->str_default_material && !found)
						{							
							m_currentSceneData->textures.push_back(t);
							TextureData td;
							td.path = path;
							td.filter = t->getFilter();
							m_currentSceneData->textureData.push_back(td);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear##Roughness")) { removeExistTexture(mat->getRoughnessTexture(), m_currentSceneData, m_pc->textureManager, gdm); mat->setRoughnessTexture(nullptr); }

					ImGui::Text("Normal     ");
					ImGui::SameLine();
					ImGui::Image((ImTextureID)mat->getNormalTexture()->getTextureID(), ImVec2(80, 80));
					if (ImGui::IsItemClicked(0))
					{
						for (int i = 0; i < m_currentSceneData->textures.size(); i++)
						{
							if (m_currentSceneData->textures[i] == mat->getNormalTexture())
							{
								m_currentProjectLocation = m_baseProjectLocation + removeFilename(m_currentSceneData->textureData[i].path);
							}
						}
					}
					path = dropTargetImage();
					if (!path.empty())
					{
						removeExistTexture(mat->getNormalTexture(), m_currentSceneData, m_pc->textureManager, gdm);
						bool found = false;
						Textures* t = nullptr;
						for (int i = 0; i < m_currentSceneData->textureData.size(); i++)
						{
							if (m_currentSceneData->textureData[i].path == path && m_currentSceneData->textureData[i].filter == filter)
							{
								t = m_currentSceneData->textures[i];								
								found = true;
							}
						}
						if (!found)
						{
							t = m_pc->textureManager->createTexture((m_baseProjectLocation+path).c_str(), filter);
						}
						mat->setNormalTexture(t);
						if (mat != m_gdm->str_default_material && !found)
						{							
							m_currentSceneData->textures.push_back(t);
							TextureData td;
							td.path = path;
							td.filter = t->getFilter();
							m_currentSceneData->textureData.push_back(td);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear##Normal")) 
					{ 
						removeExistTexture(mat->getNormalTexture(), m_currentSceneData, m_pc->textureManager, gdm); 
						mat->setNormalTexture(nullptr); 
					}
					ImGui::SameLine();
					if (ImGui::Button("Create from Albedo"))
					{
						if(gdm->str_default_texture != mat->getAlbedoTexture())
						{
							mat->setNormalTexture(m_pc->textureManager->generateNormal(mat->getAlbedoTexture(), mat->getAlbedoTexture()->getFilter()));
						}						
					}

					ImGui::Text("Oclusion   ");
					ImGui::SameLine();
					ImGui::Image((ImTextureID)mat->getOclusionTexture()->getTextureID(), ImVec2(80, 80));
					if (ImGui::IsItemClicked(0))
					{
						for (int i = 0; i < m_currentSceneData->textures.size(); i++)
						{
							if (m_currentSceneData->textures[i] == mat->getOclusionTexture())
							{
								m_currentProjectLocation = m_baseProjectLocation + removeFilename(m_currentSceneData->textureData[i].path);
							}
						}
					}
					path = dropTargetImage();
					if (!path.empty())
					{
						removeExistTexture(mat->getOclusionTexture(), m_currentSceneData, m_pc->textureManager, gdm);
						bool found = false;
						Textures* t = nullptr;
						for (int i = 0; i < m_currentSceneData->textureData.size(); i++)
						{
							if (m_currentSceneData->textureData[i].path == path && m_currentSceneData->textureData[i].filter == filter)
							{
								t = m_currentSceneData->textures[i];
								found = true;
							}
						}
						if (!found)
						{
							t = m_pc->textureManager->createTexture((m_baseProjectLocation+path).c_str(), filter);
						}
						mat->setOclusionTexture(t);
						if (mat != m_gdm->str_default_material && !found)
						{							
							m_currentSceneData->textures.push_back(t);
							TextureData td;
							td.path = path;
							td.filter = t->getFilter();
							m_currentSceneData->textureData.push_back(td);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear##Oclusion")) { removeExistTexture(mat->getOclusionTexture(), m_currentSceneData, m_pc->textureManager, gdm); mat->setOclusionTexture(nullptr); }					
				}
			}
			const std::vector<std::string>& derivedName = FACTORY(Behaviour).getDerivedName();
			if (m_scriptCombo < derivedName.size())
			{
				if (ImGui::Button("Add Script"))
				{					
					ScriptData sd;
					sd.pathName = derivedName[m_scriptCombo];
					Behaviour* b = (FACTORY(Behaviour).create(sd.pathName));
					sd.data = b->serialize();
					delete b;
					scriptObject[m_selectedOBJ].push_back(sd);
				}
				ImGui::SameLine();
				if (ImGui::Button("Remove Script"))
				{
					std::vector<ScriptData>& lso = scriptObject[m_selectedOBJ];
					std::string sc = derivedName[m_scriptCombo];
					lso.erase(
						std::remove_if(lso.begin(), lso.end(),
							[&sc](const ScriptData& sd) { return sd.pathName == sc; }),
						lso.end()
					);
				}
				ImGui::SameLine();
				if (ImGui::BeginCombo("##ScriptCombo", derivedName[m_scriptCombo].c_str()))
				{
					for (int n = 0; n < derivedName.size(); n++)
					{
						const bool is_selected = (m_scriptCombo == n);
						if (ImGui::Selectable(derivedName[n].c_str(), is_selected))
						{
							m_scriptCombo = n;
						}

						if (is_selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}
			}
			std::vector<ScriptData>& scriptUse = scriptObject[m_selectedOBJ];
			ImGui::BeginChild("##ScriptCurrent", ImVec2(0, 200), true);
			for (int i = 0; i < scriptUse.size(); i++)
			{
				ImGui::TextColored(m_colRGB, scriptUse[i].pathName.c_str());
				strncpy(m_scriptBuffer, scriptUse[i].data.c_str(), 4096);
				m_scriptBuffer[4096 - 1] = '\0';
				if (ImGui::InputTextMultiline(("##Script_" + std::to_string(i)).c_str(), m_scriptBuffer, 4096,ImVec2(500,200)))
				{
					scriptUse[i].data = std::string(m_scriptBuffer);
				}
			}

			ImGui::EndChild();
			std::vector<CollisionData>& collisionUse = collisionObj[m_selectedOBJ];
			
			if (ImGui::BeginCombo("##CollisionCombo", collisionType[m_collisionCombo]))
			{
				for (int n = 0; n < collisionType.size(); n++)
				{					
					const bool is_selected = (m_collisionCombo == n);
					if (ImGui::Selectable(collisionType[n], is_selected))
					{
						m_collisionCombo = n;
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
			ImGui::DragInt("SubDivid", &m_subDividCollision,1.0f,1);
			if (ImGui::Button("Create Collision"))
			{
				CollisionData data;
				
				if (Model* model = dynamic_cast<Model*>(m_selectedOBJ))
				{
					if (m_collisionCombo == 0)
					{
						glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
						glm::vec3 max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
						const std::vector<Vertex>& vertex = ((ShapeBufferBase*)model->getShapeBuffer())->getVertices();
						glm::mat4 mat = glm::scale(glm::mat4(1.0f), model->getScale());						
						for (int j = 0; j < vertex.size(); j++)
						{
							glm::vec3 np = glm::vec3(mat * glm::vec4(vertex[j].pos, 1));
							min.x = min.x < np.x ? min.x : np.x;
							min.y = min.y < np.y ? min.y : np.y;
							min.z = min.z < np.z ? min.z : np.z;

							max.x = max.x > np.x ? max.x : np.x;
							max.y = max.y > np.y ? max.y : np.y;
							max.z = max.z > np.z ? max.z : np.z;
						}
						glm::vec3 centerCollision = glm::vec3(glm::translate(glm::mat4(1.0f), model->getPosition()) * glm::toMat4(glm::quat(model->getRotation())) * glm::vec4(((max + min) / 2.0f),1));
						glm::vec3 extend = (max - min)/2.0f;
						data.position = centerCollision;
						data.data = glm::vec4(extend,0);
						data.euler = model->getEulerAngles();
					}
					else if (m_collisionCombo == 1)
					{
						data.position = m_selectedOBJ->getPosition();
						data.data = glm::vec4(1, 1, 1, 1);
						data.euler = glm::vec3(0, 0, 0);
					}
					else
					{
						data.position = m_selectedOBJ->getPosition();
						data.data = glm::vec4(1, 1, 1, 1);
						data.euler = glm::vec3(0, 0, 0);
					}
				}				
				else
				{
					data.position = m_selectedOBJ->getPosition();
					data.type = m_collisionCombo;
					data.data = glm::vec4(1, 1, 1, 1);
				}
				data.type = m_collisionCombo;
				data.mass = 1.0f;
				collisionObj[m_selectedOBJ].push_back(data);
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear Collision"))
			{
				collisionObj[m_selectedOBJ].clear();
			}
			ImGui::BeginChild("Collision Group", ImVec2(0, 200), true);

			for (int i = 0; i < collisionUse.size(); i++)
			{
				ImGui::DragFloat3(("Position##Col_"+std::to_string(i)).c_str(), &collisionUse[i].position[0]);
				if (collisionUse[i].type == 0)
				{
					ImGui::DragFloat3(("Extend##Col_" + std::to_string(i)).c_str(), &collisionUse[i].data[0]);
				}
				else if (collisionUse[i].type == 1)
				{
					ImGui::DragFloat(("radius##Col_" + std::to_string(i)).c_str(), &collisionUse[i].data.x);
				}
				else if (collisionUse[i].type == 2)
				{
					ImGui::DragFloat(("radius##Col_" + std::to_string(i)).c_str(), &collisionUse[i].data.x);
					ImGui::DragFloat(("Height##Col_" + std::to_string(i)).c_str(), &collisionUse[i].data.y);
				}
				ImGui::DragFloat3(("Euler##Col_" + std::to_string(i)).c_str(), &collisionUse[i].euler[0]);
				ImGui::DragFloat(("Mass##Col_" + std::to_string(i)).c_str(), &collisionUse[i].mass);
				ImGui::Separator();
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}

	if (m_editorData->project)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Project", &m_editorData->project);
		if (m_currentProjectData != nullptr)
		{			
			drawFolderContents(m_currentProjectData->assetPath, m_currentProjectLocation);
		}
		ImGui::End();
	}
	if (m_editorData->postProcess)
	{
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
		ImGui::Begin("Post Process", &m_editorData->postProcess);
		PPSetting* setting = m_pc->postProcess->getPPSetting();
		ImGui::Text("Post processing");		
		m_ambient = m_pc->settingManager->getAmbient();
		if (ImGui::DragFloat("Ambient", &m_ambient))
		{
			m_pc->settingManager->setAmbient(m_ambient);			
		}
		if(ImGui::DragFloat("Gamma", setting->gamma))
		{
			if (m_currentSceneData != nullptr) { m_currentSceneData->pp.gamma = setting->gamma; }
		}
		if (ImGui::DragFloat("Exposure##Bloom", &setting->exposure, 0.1f))
		{
			if (m_currentSceneData != nullptr) { m_currentSceneData->pp.exposure = setting->exposure; }
		}
		if (ImGui::TreeNodeEx("Bloom"))
		{
			if (ImGui::Checkbox("Active##Bloom", &setting->bloom))
			{
				if (m_currentSceneData != nullptr) { m_currentSceneData->pp.bloom = setting->bloom; }
			}
			if (ImGui::DragFloat("Filter##Bloom", &setting->bloom_filter,0.01f))
			{
				if (m_currentSceneData != nullptr) { m_currentSceneData->pp.bloom_filter = setting->bloom_filter; }
			}
			if (ImGui::DragFloat("Threshold##Bloom", &setting->bloom_threshold, 0.01f))
			{
				if (m_currentSceneData != nullptr) { m_currentSceneData->pp.bloom_threshold = setting->bloom_threshold; }
			}
			if (ImGui::DragFloat("Intensity##Bloom", &setting->bloom_intensity, 0.01f))
			{
				if (m_currentSceneData != nullptr) { m_currentSceneData->pp.bloom_intensity = setting->bloom_intensity; }
			}
			ImGui::TreePop();			
		}
		
		ImGui::End();
	}
	if (m_editorData->hiearchy)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Hiearchy", &m_editorData->hiearchy);
		if (m_currentSceneData != nullptr)
		{
			ImGui::BeginChild("##HiearchyBP", ImVec2(0, 50), true);
			ImGui::TextColored(m_colRGB, m_currentSceneData->name.c_str());
			ImGui::SameLine();
			if (ImGui::ImageButton(m_icon[8], ImVec2(m_iconMoveSize, m_iconMoveSize))) { op = ImGuizmo::TRANSLATE; }
			ImGui::SameLine();
			if (ImGui::ImageButton(m_icon[9], ImVec2(m_iconMoveSize, m_iconMoveSize))) { op = ImGuizmo::ROTATE; }
			ImGui::SameLine();
			if (ImGui::ImageButton(m_icon[10], ImVec2(m_iconMoveSize, m_iconMoveSize))) { op = ImGuizmo::SCALE; }
			ImGui::SameLine();
			if (ImGui::ImageButton(m_icon[11], ImVec2(m_iconMoveSize, m_iconMoveSize))) { op = ImGuizmo::TRANSLATE | ImGuizmo::ROTATE; }
			ImGui::SameLine();
			//m_pc->physicsEngine
			if (ImGui::Checkbox("Guizmo", &m_guizmo))
			{
				if (m_guizmo)
				{
					m_pc->physicsEngine->DebugDrawCollider();
				}
				else
				{
					m_pc->physicsEngine->DebugClearCollider();
				}
			}
			ImGui::EndChild();
			drawTreeView();			
		}
		ImGui::End();
		if (m_selectedOBJ != nullptr)
		{
			Camera* cam = m_pc->cameraManager->getCurrentCamera();
			glm::mat4 view = cam->getViewMatrix();
			glm::mat4 proj = cam->getProjectionMatrix();
			glm::mat4 objMod = m_selectedOBJ->getModelMatrix();
			ImGuizmo::SetRect(mainWindowPosX, mainWindowPosY, m_pc->settingManager->getWindowWidth(), m_pc->settingManager->getWindowHeight());
			ImGuizmo::SetGizmoSizeClipSpace(0.08f);
			if (ImGuizmo::Manipulate(&view[0][0], &proj[0][0], (ImGuizmo::OPERATION)op, ImGuizmo::WORLD, &objMod[0][0]))
			{				
				glm::vec3 pos;
				glm::vec3 eul;
				glm::vec3 scale;
				ImGuizmo::DecomposeMatrixToComponents(&objMod[0][0], &pos[0], &eul[0], &scale[0]);
				m_selectedOBJ->setPosition(pos);
				m_selectedOBJ->setEulerAngles(eul);
				m_selectedOBJ->setScale(scale);
			}
			if (m_pc->inputManager->getKeyDown(GLFW_KEY_V))
			{
				glm::vec3 pos = m_pc->renderingEngine->getWorldCoordinates(m_pc->inputManager->getMousePosX(), m_pc->inputManager->getMousePosY());
				for (int i = 0; i < m_currentSceneData->models.size(); i++)
				{
					if (m_selectedOBJ == m_currentSceneData->models[i])
					{
						const std::vector<Vertex>&  vertex = ((ShapeBufferBase*)m_currentSceneData->models[i]->getShapeBuffer())->getVertices();
						glm::mat4 mat = m_selectedOBJ->getModelMatrix();
						m_offsetMove = m_selectedOBJ->getPosition();
						float dist = glm::distance(m_offsetMove, pos);
						for (int j = 0; j < vertex.size(); j++)
						{							
							glm::vec3 np = glm::vec3(mat* glm::vec4(vertex[j].pos,1));
							if (glm::distance(np, pos) < dist)
							{
								m_offsetMove = np;
								dist = glm::distance(np, pos);
							}
						}
						m_offsetMove = m_offsetMove-m_selectedOBJ->getPosition();						
						break;
					}
				}
				if (m_pc->inputManager->getKey(GLFW_KEY_C))
				{
					float dist = FLT_MAX;
					int idModel = -1;
					int idVertex = -1;
					for (int i = 0; i < m_currentSceneData->models.size(); i++)
					{
						if (m_selectedOBJ != m_currentSceneData->models[i])
						{
							const std::vector<Vertex>& vertex = ((ShapeBufferBase*)m_currentSceneData->models[i]->getShapeBuffer())->getVertices();
							glm::mat4 mat = m_currentSceneData->models[i]->getModelMatrix();
							for (int j = 0; j < vertex.size(); j++)
							{
								glm::vec3 np = glm::vec3(mat * glm::vec4(vertex[j].pos, 1));
								if (glm::distance(np, pos) < dist)
								{									
									dist = glm::distance(np, pos);
									idModel = i;
									idVertex = j;
								}
							}
						}
					}
					if (idModel != -1)
					{
						const std::vector<Vertex>& vertex = ((ShapeBufferBase*)m_currentSceneData->models[idModel]->getShapeBuffer())->getVertices();
						glm::mat4 mat = m_currentSceneData->models[idModel]->getModelMatrix();
						glm::vec3 np = glm::vec3(mat * glm::vec4(vertex[idVertex].pos, 1));
						m_selectedOBJ->setPosition(np - m_offsetMove);
					}
				}
			}
			if (m_pc->inputManager->getKey(GLFW_KEY_V) && !m_pc->inputManager->getKey(GLFW_KEY_C))
			{				
				glm::vec3 pos = m_pc->renderingEngine->getWorldCoordinates(m_pc->inputManager->getMousePosX(), m_pc->inputManager->getMousePosY());
				if (glm::distance(cam->getPosition(), pos) < cam->getFar())
				{
					m_selectedOBJ->setPosition(pos- m_offsetMove);
				}						
			}
		}	
		if (m_currentSceneData != nullptr && m_generatePathFindingNextPlay && m_playMode == 1)
		{
			if (m_pc->inputManager->getKey(GLFW_KEY_K) && !m_currentSceneData->path.has)
			{
				m_currentSceneData->path.has = true;
				Debug::Log("Scene Add Current path");
			}
		}
		if (m_pc->inputManager->getMouse(GLFW_MOUSE_BUTTON_LEFT) && m_currentSceneData != nullptr)
		{
			if (!m_clickedSceneSelected && !ImGuizmo::IsUsing() && !ImGui::GetIO().WantCaptureMouse)
			{
				Camera* cam = m_pc->cameraManager->getCurrentCamera();
				glm::vec3 pos = m_pc->renderingEngine->getWorldCoordinates(m_pc->inputManager->getMousePosX(), m_pc->inputManager->getMousePosY());
				if (glm::distance(cam->getPosition(), pos) < cam->getFar())
				{
					float dist = FLT_MAX;
					float nd;
					int selectedModel = -1;
					for (int i = 0; i < m_currentSceneData->models.size(); i++)
					{
						nd = glm::distance(m_currentSceneData->models[i]->getPosition(), pos);
						if (nd < dist)
						{
							dist = nd;
							selectedModel = i;
						}
					}
					if (selectedModel != -1)
					{
						m_selectedOBJ = m_currentSceneData->models[selectedModel];
					}
				}
				else
				{
					m_selectedOBJ = nullptr;
				}
			}
			m_clickedSceneSelected = true;
		}
		else
		{
			m_clickedSceneSelected = false;
		}
	}
}