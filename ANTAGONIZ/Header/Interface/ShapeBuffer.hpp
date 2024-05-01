#ifndef __ENGINE_MODEL_BUFFER_INTERFACE__
#define __ENGINE_MODEL_BUFFER_INTERFACE__

namespace Ge
{
	class ShapeBuffer
	{
	public:
		unsigned int getVAO() const
		{
			return m_VAO;
		}

		unsigned int getVBO() const
		{
			return m_VBO;
		}

		unsigned int getIBO() const
		{
			return m_IBO;
		}
		virtual void SetupVAO(unsigned int program) = 0;
		virtual unsigned int getIndiceSize() const = 0;
	protected:
		unsigned int m_VAO;
		unsigned int m_VBO;
		unsigned int m_IBO;
	};
}
#endif //!__ENGINE_MODEL_BUFFER_INTERFACE__