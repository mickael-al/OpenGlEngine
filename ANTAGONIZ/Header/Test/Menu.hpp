#ifndef __MENU__
#define __MENU__

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "InitScene.hpp"

class Menu : public Scene, public ImguiBlock, public Behaviour
{
public:
	void load();
	void unload();
	void preRender(VulkanMisc* vM);
	void render(VulkanMisc* vM);
	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
private:
	InitScene m_is;
	ptrClass m_pc;
	std::vector<Textures*> m_textures;
	std::vector<VkDescriptorSet> m_descripteurs;
	std::vector<bool> m_menuHover;
	ImGuiWindowFlags m_allFlags;
	FontData* m_font;
	FontData* m_fontTitel;
	ImVec4 m_titreColor;
	ImVec4 m_textColor;
	bool m_changeScene = false;
};

#endif //!__MENU__