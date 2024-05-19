#ifndef __ENGINE_MODEL_SHAPE__
#define __ENGINE_MODEL_SHAPE__

#include "GObject.hpp"
#include "UniformBufferObject.hpp"

namespace Ge
{
	class Materials;
	class ShapeBuffer;
}

struct GraphicsDataMisc;

namespace Ge
{
	class Model : public GObject
	{
	public:
		Model(ShapeBuffer * buffer, unsigned int index, GraphicsDataMisc * gdm);
		~Model();		
		ShapeBuffer * getShapeBuffer() const;
		UniformBufferObject getUBO() const;
		void setMaterial(Materials * m);
		Materials * getMaterial() const;
		void mapMemory() override;
		void setIndex(unsigned int index);
		unsigned int getIndex();
		void onGUI();
	private:
		GraphicsDataMisc * m_gdm;
		ShapeBuffer * m_buffer;
		Materials * m_material;
		UniformBufferObject m_ubo{};
		unsigned int m_index;
		unsigned int m_ssbo;
	};
}

#endif //!__ENGINE_MODEL_SHAPE__
