#ifndef __EDITOR__
#define __EDITOR__

#include "Behaviour.hpp"
#include "ImguiBlock.hpp"
#include "ProjectData.hpp"
#include <map>
#define SPAWN_DISTANCE 5.0f

enum LoadSceneType
{
	Override,
	Additive
};

enum ObjectType
{
	SceneOBJ,
};

struct ptrClass;
namespace Ge
{
	class Camera;
	class CollisionWraper;
	class Textures;
	class CollisionShape;
	class Editor final : public Behaviour, public ImguiBlock
	{
	public:
		Editor();
		static Editor* instance();
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
		void loadSceneGame(const std::string& filePath, LoadSceneType type);
		GraphiquePipeline* getPipelineByScene(const std::string& Name);
		std::vector<Materials*> getMaterialByScene(const std::string& Name);
		void loadScene(const std::string& filePath, SceneData* sd);		
		void saveScene(const std::string& filePath, SceneData* sd);
		void addModelToScene(const std::string& filePath, bool fbx = false);
		void addAudioToScene(const std::string& filePath);
		void addMaterialToModel(Model * obj);
		void addDuplicateMaterialToModel(Model* obj);
		void addLightToScene(int type);
		void addEmptyToScene();
		void playScene();
		void stopScene();
		void globalSave();
		void clearCurrentProject();
		std::string dropTargetImage();
		void init(GraphicsDataMisc* gdm);
		void render(GraphicsDataMisc* gdm);
	private:
		const ptrClass * m_pc;
		static Editor* s_instance;
		GraphicsDataMisc* m_gdm;
		std::vector<ImTextureID> m_icon;
		EditorConfig* m_editorData = nullptr;
		ProjectData* m_currentProjectData = nullptr;
		ImVec4 m_colRGB = ImVec4(0,0,0,1);
		bool m_newProjectModal = false;
		bool m_openProjectModal = false;
		bool m_openPathFinding = false;
		bool m_openPathShader = false;
		bool m_createObject = false;
		bool m_deleteOject = false;		
		bool m_guizmo = false;
		bool m_hide = false;
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
		char m_pathFindingName[256];
		char m_scriptBuffer[4096];
		float m_ambient = 0.1f;
		ShaderData currentWriteSD;

		int m_playMode = 0;
		int m_scriptCombo = 0;
		int m_collisionCombo = 0;
		int m_subDividCollision = 1;
		std::map<GObject*, std::vector<ScriptData>> scriptObject;
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
		
		bool m_generatePathFindingNextPlay = false;
		bool m_gameChangeScene = false;
		float m_currentFovCS;
		Camera* m_tempFlyCamera;

		//Project		
		int m_iconSize = 64;
		int m_iconModeSize = 32;
		int m_iconMoveSize = 16;
		int m_columns = 8;
		std::string m_currentProjectLocation;
		std::string m_baseProjectLocation;
		SceneData* m_currentSceneData = nullptr;
	};
}
#endif //!__EDITOR__