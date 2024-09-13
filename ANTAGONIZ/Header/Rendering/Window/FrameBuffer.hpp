#ifndef __FRAME_BUFFER__
#define __FRAME_BUFFER__

#include "Initializer.hpp"

namespace Ge
{
	class FrameBuffer final : public InitializerAPI
	{
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc* gdm);		
		unsigned int getFrameBuffer() const;
		unsigned int getPosition() const;
		unsigned int getNormal() const;
		unsigned int getColor() const;
		unsigned int getOther() const;
		unsigned int getDepth() const;
		unsigned int getColorFoward() const;		
		unsigned int getFowardFrameBuffer() const;
		void release();
	protected:
		friend class Window;
		void resize(int width, int height);
	private:
		unsigned int m_framebuffer;
		unsigned int m_gPosition;
		unsigned int m_gNormal;
		unsigned int m_gColor;
		unsigned int m_gOther;
		unsigned int m_gDepth;
		GraphicsDataMisc* m_gdm;

		unsigned int m_framebufferFoward;
		unsigned int m_fColor;
	};
}
#endif // !__FRAME_BUFFER__
