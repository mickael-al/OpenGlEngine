#ifndef __ENGINE_LIGHT__
#define __ENGINE_LIGHT__

#include "GObject.hpp"
#include "UniformBufferLight.hpp"
#include "ShadowMatrix.hpp"

#define TEXTURE_DIM 1024
#define SHADOW_MAP_CASCADE_COUNT 4
#define SHADOW_MAP_CUBE_COUNT 6
#define SHADOW_MAP_SPOT_COUNT 4

struct GraphicsDataMisc;

namespace Ge
{
	class Lights : public GObject
	{
	public:
		Lights(unsigned int index, GraphicsDataMisc *gdm);		
		glm::vec3 getColors() const;
		float getRange() const;
		float getSpotAngle() const;
		int getStatus() const; //Statut directional spotlight pointlight
		unsigned int getIndex() const;
		unsigned int getShadowIndex() const;
		void setColors(glm::vec3 color);
		void setRange(float r);
		void setSpotAngle(float r);
		void setIndex(unsigned int i);
		void setShadowIndex(unsigned int i);
		void mapMemory();
		void mapMemoryShadow();
		void setshadow(bool state);
		bool getshadow() const;
		void onGUI() override;
		virtual ~Lights();		
	protected:
		UniformBufferLight m_ubl{};
		std::vector<ShadowMatrix> m_shadowMatrix;
		GraphicsDataMisc * m_gdm;
		unsigned int m_index = 0;
		unsigned int m_ssbo;
		unsigned int m_ssboShadow;
		bool m_shadow = false;
		//shadow Dir
		float cascadeSplitLambda = 0.85f;
		glm::vec3 frustumCorners[8];
	};
}

#endif //__ENGINE_LIGHT__
