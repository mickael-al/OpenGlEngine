#ifndef __ENGINE_MATERIAL_MANAGER__
#define __ENGINE_MATERIAL_MANAGER__

#include "Initializer.hpp"
#include "Manager.hpp"
#include <vector>

namespace Ge
{
	class TextureManager;
	class Materials;
}

namespace Ge
{
	class MaterialManager final : public InitializerAPI, public Manager
    {
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc * gdm);
		void release();
		const std::vector<Materials*>& getMaterials() const;
		void updateStorage();
		friend class Materials;
		void updateMaterialExecutionOrder();
    public:
        Materials * createMaterial();
		Materials * getDefaultMaterial();
        std::vector<Materials*> & loadMltMaterial(const char* path, bool filter, TextureManager * tm);
        void destroyMaterial(Materials * material);				
    private:
		Materials * m_defaultMaterial;
        std::vector<Materials *> m_materials;
		GraphicsDataMisc * m_gdm;
    };
}

#endif //!__ENGINE_MATERIAL_MANAGER__