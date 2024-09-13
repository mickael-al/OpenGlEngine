#ifndef __ENGINE_MATERIALS__
#define __ENGINE_MATERIALS__

#include <glm/glm.hpp>
#include "UniformBufferMaterial.hpp"
#include "Component.hpp"
#include <vector>

namespace Ge
{
	class Textures;
	class GraphiquePipeline;
	class Model;	
}

struct ptrClass;
struct GraphicsDataMisc;

namespace Ge
{
	class Materials : public Component
	{
	public:
		Materials(unsigned int index, GraphicsDataMisc * gdm);
		~Materials();
		void setColor(glm::vec4 color);
		void setMetallic(float metal);
		void setRoughness(float roughness);
		void setNormal(float normal);
		void setOclusion(float ao);
		void setEmission(float emit);
		void setAlbedoTexture(Textures * albedo);
		void setNormalTexture(Textures * normal);
		void setMetallicTexture(Textures * metallic);
		void setRoughnessTexture(Textures * roughness);
		void setOclusionTexture(Textures * oclu);
		void setOffset(glm::vec2 off);
		void setTilling(glm::vec2 tilling);
		void setPipeline(GraphiquePipeline * p);		
		void setIndex(int i);
		int getIndex() const;
		glm::vec4 getColor() const;
		float getMetallic() const;
		float getRoughness() const;
		float getNormal() const;
		float getOclusion() const;
		float getEmission() const;
		Textures * getAlbedoTexture() const;
		Textures * getNormalTexture() const;
		Textures * getMetallicTexture() const;
		Textures * getRoughnessTexture() const;
		Textures * getOclusionTexture() const;		
		glm::vec2 getOffset() const;
		glm::vec2 getTilling() const;
		GraphiquePipeline * getPipeline() const;
		std::vector<unsigned int> & getAditionalSSBO();
		unsigned int getAditionalInstanced() const;		
		void setAditionalInstanced(unsigned int instance);
		void updateUniformBufferMaterial();
		void onGUI();
		bool getDraw() const;		
		void setDraw(bool draw);
		bool getCastShadow() const;
		void setCastShadow(bool cs);
		bool * getDrawAddr();
		void setDepthTest(bool state);
		bool getDepthTest() const;
		void setDepthWrite(bool state);
		bool getDepthWrite() const;
	public:
		friend class Model;
		void addModel(Model * model);
		void removeModel(Model * model);
	private:
		const ptrClass * m_pc;
		bool m_draw = true;
		bool m_castShadow = true;
		bool m_depthTest = true;
		bool m_depthWrite = true;
		UniformBufferMaterial m_ubm{};
		GraphicsDataMisc * m_gdm;
		Textures * m_albedoMap;
		Textures * m_normalMap;
		Textures * m_metallicMap;
		Textures * m_RoughnessMap;
		Textures * m_aoMap;
		GraphiquePipeline * m_pipeline;
		std::vector<Model*> m_model;
		std::vector<unsigned int> m_aditionalSSBO;
		unsigned int m_aditionalInstanced = 0;
		unsigned int m_ssbo;
		unsigned int m_index = 0;
	};
}

#endif //!__ENGINE_MATERIALS__