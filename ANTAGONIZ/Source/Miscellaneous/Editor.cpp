#include "Editor.hpp"
#include "PointeurClass.hpp"
#include "Engine.hpp"
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
#include <sys/stat.h>
#include <filesystem>

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

void dtv(GObject * obj,int * id)
{
	ImGui::PushID(*id);
	if (ImGui::TreeNode(obj->getName()->c_str()))
	{		
		const std::vector<GObject*>& child = obj->getChilds();	
		for (int i = 0; i < child.size(); i++)
		{
			(*id)++;
			dtv(child[i], id);
		}
		ImGui::TreePop();
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
		else
		{
			if (ImGui::ImageButton(m_icon[5], ImVec2(m_iconSize, m_iconSize)))
			{

			}
		}
		ImGui::PopID();
		ImGui::TextWrapped("%s", entry.path().filename().string().c_str());
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



void Editor::start()
{
	m_pc = Engine::getPtrClassAddr();
	m_pc->hud->addImgui(this);
	m_editorData = new EditorConfig();
	PathManager::getHomeDirectory();
	loadConfig(PathManager::getHomeDirectory()+"/config.json" , m_editorData);
	for (int i = 0; i < 9; i++)
	{
		std::string fn = "../Asset/Editor/" + std::to_string(i) + ".png";
		m_icon.push_back((ImTextureID)m_pc->textureManager->createTexture(fn.c_str())->getTextureID());
	}

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
	m_currentProjectLocation = ec->assetPath;
}

void Editor::saveProject(const std::string& filePath, ProjectData* ec)
{
	std::string projectPath= filePath + "\\" + ec->projetName;
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
	sd->currentPath = filePath;
	m_currentProjectData->lastSceneOpen = filePath;

	sd->shader.clear();
	for (int i = 0; i < sd->shaderData.size(); i++)
	{		
		GraphiquePipeline* gp = m_pc->graphiquePipelineManager->createPipeline(sd->shaderData[i].frag, sd->shaderData[i].vert, sd->shaderData[i].back, sd->shaderData[i].multiS, sd->shaderData[i].transparency, sd->shaderData[i].cullmode);
		sd->shader.push_back(gp);
	}
	sd->textures.clear();
	for (int i = 0; i < sd->textureData.size(); i++)
	{
		sd->textures.push_back(m_pc->textureManager->createTexture(sd->textureData[i].path.c_str(), sd->textureData[i].filter));
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
		sd->buffer.push_back(m_pc->modelManager->allocateBuffer(sd->bufferData[i].path.c_str(), sd->bufferData[i].normalRecalculate));
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
			if (sd->modelData[i].idMaterial >= 0)
			{
				m->setMaterial(sd->materials[sd->modelData[i].idMaterial]);
			}
			sd->models.push_back(m);
		}
	}
	Camera* cam = m_pc->cameraManager->getCurrentCamera();
	cam->setPosition(sd->freeCamPos);
	cam->setRotation(sd->freeCamRot);
}

void Editor::globalSave()
{
	if (m_currentProjectData != nullptr)
	{
		saveProject(m_currentProjectData->projetPath, m_currentProjectData);
		if (m_currentSceneData != nullptr)
		{
			saveScene(m_currentSceneData->currentPath, m_currentSceneData);
		}
	}
}

void Editor::addModelToScene(const std::string& filePath)
{
	if (m_currentSceneData != nullptr)
	{
		ShapeBuffer* sb = nullptr;
		bool normalR = false;
		for (int i = 0; i < m_currentSceneData->bufferData.size(); i++)
		{			
			if (m_currentSceneData->bufferData[i].path == filePath)
			{
				sb = m_currentSceneData->buffer[i];				
				normalR = m_currentSceneData->bufferData[i].normalRecalculate;
				i = m_currentSceneData->bufferData.size();
			}
		}		
		if (sb == nullptr)
		{
			sb = m_pc->modelManager->allocateBuffer(filePath.c_str(), normalR);
			m_currentSceneData->buffer.push_back(sb);
			BufferData bd;
			bd.normalRecalculate = normalR;
			bd.path = filePath.c_str();
			m_currentSceneData->bufferData.push_back(bd);
		}
		Model* m = m_pc->modelManager->createModel(sb);
		Camera * cam = m_pc->cameraManager->getCurrentCamera();
		m->setPosition(cam->getPosition() + cam->transformDirectionAxeZ() * 5.0f);
		m_currentSceneData->models.push_back(m);
	}
}

void Editor::saveScene(const std::string& filePath, SceneData* sd)
{
	Camera * cam = m_pc->cameraManager->getCurrentCamera();
	sd->freeCamPos = cam->getPosition();
	sd->freeCamRot = cam->getRotation();
	
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
		
		for (int j = 0; j < sd->textures.size(); i++)
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
		for (int j = 0; j < sd->shader.size(); i++)
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
	m_colRGB = HSVtoRGB(fmod(m_pc->time->getTime()*36.0f,360.0f), 1.0f, 1.0f);
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
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.64f);		
}

void Editor::render(GraphicsDataMisc* gdm)
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 wbg = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_NoDockingOverCentralNode);	
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
	if (m_pc->inputManager->getKeyDown(GLFW_KEY_S) && m_pc->inputManager->getKey(GLFW_KEY_LEFT_CONTROL))
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
			ImGui::EndMenu();
		}		
		if (m_currentProjectData != nullptr)
		{
			ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(m_currentProjectData->projetName).x-15);
			ImGui::TextColored(m_colRGB,m_currentProjectData->projetName);
		}
		ImGui::EndMainMenuBar();
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
						loadScene(m_currentProjectData->lastSceneOpen, m_currentSceneData);
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
					m_currentProjectData->lastSceneOpen = m_currentProjectLocation + "\\Scene\\Main.scene";
					saveProject(m_currentProjectData->projetPath, m_currentProjectData);
					if (!copyAndRenameDirectory("..\\Asset\\Model", m_currentProjectData->assetPath + "\\Ressources\\Model\\BaseModel"))
					{
						Debug::Error("Failed to copy Base Model");
					}
					SceneData* sd = new SceneData();
					m_currentSceneData = sd;
					saveScene(m_currentProjectData->lastSceneOpen, m_currentSceneData);
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
							saveScene(m_currentSceneData->currentPath, m_currentSceneData);
							clearScene(m_currentSceneData);
							delete m_currentSceneData;
							m_currentSceneData = nullptr;
						}
						SceneData* sd = new SceneData();
						m_currentSceneData = sd;
						sd->name = m_objectName;
						sd->currentPath = m_currentProjectLocation + "\\" + m_objectName + ".scene";
						m_currentProjectData->lastSceneOpen = sd->currentPath;
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
		if (ImGui::ImageButton(m_icon[6], ImVec2(m_iconModeSize, m_iconModeSize)))
		{

		}
		ImGui::SameLine();
		if (ImGui::ImageButton(m_icon[7], ImVec2(m_iconModeSize, m_iconModeSize)))
		{

		}
		ImGui::SameLine();
		if (ImGui::ImageButton(m_icon[8], ImVec2(m_iconModeSize, m_iconModeSize)))
		{

		}		
		style.Colors[ImGuiCol_Button] = colbbg;
		ImGui::SameLine();
		ImGui::End();
	}

	if (m_editorData->inspector)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Inspector", &m_editorData->inspector);
		
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

	if (m_editorData->hiearchy)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Hiearchy", &m_editorData->hiearchy);
		if (m_currentSceneData != nullptr)
		{
			ImGui::BeginChild("##HiearchyBP", ImVec2(0, 30), true);
			ImGui::TextColored(m_colRGB, m_currentSceneData->name.c_str());
			ImGui::EndChild();
			drawTreeView();
		}
		ImGui::End();
	}
}