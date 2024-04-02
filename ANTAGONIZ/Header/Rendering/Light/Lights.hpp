#ifndef __ENGINE_LIGHT__
#define __ENGINE_LIGHT__

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/common.hpp"
#include "GObject.hpp"

namespace Ge
{
	class Lights : public GObject
	{
	public:
		Lights(int index);
		void setColors(glm::vec3 color);
		glm::vec3 getColors() const;		
		float getRange() const;
		float getSpotAngle() const;
		int getStatus() const; //Statut directional spotlight pointlight
		bool getShadow() const;
		void setRange(float r);
		void setSpotAngle(float r);
		void setIndex(int i);
		void onGUI() override;
		void setShadow(bool state);		
		virtual ~Lights();
	protected:
		float m_nearPlane = 1.0f;
		float m_farPlane = 7.5f;
		int m_index = 0;
		bool m_shadow = false;
	};
}

#endif //__ENGINE_LIGHT__
