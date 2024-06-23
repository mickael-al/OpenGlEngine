#ifndef __EDITOR__
#define __EDITOR__

#include "Behaviour.hpp"
#include "ImguiBlock.hpp"
#include "ProjectData.hpp"
#include <map>
#include "PhysicsWraper.hpp"
#define SPAWN_DISTANCE 5.0f

enum ObjectType
{
	SceneOBJ,
};

struct ptrClass;
namespace Ge
{
	class CollisionWraper;
	class Textures;
	class Editor final : public Behaviour, public ImguiBlock
	{
	public:
		void start();
		void fixedUpdate();
		void update();
		void stop();
		void onGUI();
		void loadConfig(const std::string& filePath,EditorConfig * ec);
		void saveConfig(const std::string& filePath,EditorConfig * ec);
		void loadProject(const std::string& filePath, ProjectData* ec);
		void saveProject(const std::string& filePath, ProjectData* ec);
		bool createDirectory(const std::string& path);
		void drawFolderContents(const std::string& basePath,std::string& path);
		void drawTreeView();
		void dtv(GObject* obj, int* id);
		void deleteSceneObject(GObject * obj);
		void duplicateSceneObject(GObject* obj, GObject* parent = nullptr);
		void clearScene(SceneData* sd);
		void loadScene(const std::string& filePath, SceneData* sd);
		void saveScene(const std::string& filePath, SceneData* sd);
		void addModelToScene(const std::string& filePath);
		void addAudioToScene(const std::string& filePath);
		void addMaterialToModel(Model * obj);
		void addLightToScene(int type);
		void addEmptyToScene();
		void globalSave();
		void clearCurrentProject();
		void init(GraphicsDataMisc* gdm);
		void render(GraphicsDataMisc* gdm);
	private:
		const ptrClass * m_pc;
		GraphicsDataMisc* m_gdm;
		std::vector<ImTextureID> m_icon;
		EditorConfig* m_editorData = nullptr;
		ProjectData* m_currentProjectData = nullptr;
		ImVec4 m_colRGB = ImVec4(0,0,0,1);
		bool m_newProjectModal = false;
		bool m_openProjectModal = false;
		bool m_createObject = false;
		bool m_deleteOject = false;		
		bool m_guizmo = false;
		std::string deletePath;
		ObjectType objType;
		GObject* m_selectedOBJ = nullptr;
		std::map<int, GObject*> multiSelected;
		bool multiSelect = false;
		int m_switchTreeObj = -1;
		bool m_hasSwitch = false;
		bool m_oneClickFrame = false;
		std::string m_tempFile;
		char m_pathOpenProject[256];
		char m_objectName[256];

		int m_playMode = 0;
		int m_scriptCombo = 0;
		int m_collisionCombo = 0;
		int m_subDividCollision = 1;
		std::map<GObject*, std::vector<std::string>> scriptObject;
		std::map<GObject*, std::vector<CollisionData>> collisionObj;
		std::vector<Behaviour*> m_allBehaviourLoaded;
		std::vector<Behaviour*> m_allBehaviourPaused;
		std::vector<CollisionShape*> m_allCollisionLoaded;
		std::vector<CollisionWraper*> m_allCollisionBodyLoaded;
		std::vector<const char*> collisionType = {"Box","Sphere","Capsule"};
		//Mouse Mode
		bool m_clickedSceneSelected = false;
		glm::vec3 m_offsetMove;
		unsigned int op;
		int matCN = 0;

		//pathFinding
		bool m_activeNextPlay = true;
		glm::vec3 m_PathPosition;
		glm::vec3 m_ZoneSize = glm::vec3(10,10,10);
		glm::vec3 m_PointCount = glm::vec3(10, 10, 10);
		std::string m_pathFolder;
		float m_pathLiasonPercent = 0.2f;
		
		//Project		
		int m_iconSize = 64;
		int m_iconModeSize = 32;
		int m_iconMoveSize = 16;
		int m_columns = 8;	
		std::string m_currentProjectLocation;
		SceneData* m_currentSceneData = nullptr;
	};
}
#endif //!__EDITOR__