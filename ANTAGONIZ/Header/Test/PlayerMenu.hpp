#ifndef __PLAYER_MENU__
#define __PLAYER_MENU__

#include "GameEngine.hpp"
#include "imgui-cmake/Header/imgui.h"
#include "imgui-cmake/Header/imgui_impl_vulkan.h"

class PlayerMenu : public ImguiBlock
{
public:
	PlayerMenu();
	~PlayerMenu();
	void preRender(VulkanMisc* vM);
	void render(VulkanMisc* vM);
public:
		bool activeMenu = false;
		bool activeSetting = false;
		bool activeDialog = false;
		bool activeFight = false;
		int money = 0;
private:
	ptrClass m_pc;
	std::vector<Textures*> m_textures;
	std::vector<VkDescriptorSet> m_descripteurs;	
	ImGuiWindowFlags m_allFlags;
	FontData* m_font;
	bool m_settingBook = false;	
	bool m_settingBookHovered = false;
	bool m_cross = false;
	bool m_crossHovered = false;
	std::vector<bool> m_settingHover;
	ImVec4 m_textColor;
};

#endif //!__PLAYER_MENU__