#ifndef __ENGINE_CAMERA__
#define __ENGINE_CAMERA__

#include "GObject.hpp"
#include "UniformBufferCamera.hpp"

struct GraphicsDataMisc;


namespace Ge
{
	class LightManager;
	class Camera : public GObject
	{		
	public:
		Camera(GraphicsDataMisc * gdm,int priority);
		~Camera();
		void setFieldOfView(float fov);
		void setNear(float near);
		void setFar(float far);
		void setPriority(int p);
		void setOrtho(bool state);
		void setOrthoSize(float size);
		float getOrthoSize() const;
		bool getOrtho() const;
		float getFieldOfView();
		float getNear();
		float getFar();
		void mapMemory() override;
		int getPriority();
		float aspectRatio() const;
		glm::mat4 getViewMatrix() const;
		glm::mat4 getProjectionMatrix() const;
		void onGUI() override;
	protected:
		GraphicsDataMisc * m_gdm;
		UniformBufferCamera m_uniformBufferCamera;
		LightManager* m_lm;
		float m_fov = 80.0f;
		float m_near = 0.1f;
		float m_far = 300.0f;
		int m_priority = 0;
		bool m_ortho = false;
		float m_orthoSize;
		unsigned int m_ssbo;
	};
}

#endif //!__ENGINE_CAMERA__