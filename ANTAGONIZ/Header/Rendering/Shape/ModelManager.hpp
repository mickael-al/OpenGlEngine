#ifndef __ENGINE_MODEL_MANAGER___
#define __ENGINE_MODEL_MANAGER___

#include "Initializer.hpp"
#include "Manager.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include "MemoryManager.hpp"

namespace Ge
{
	class ShapeBuffer;
	class ShapeBufferBase;
	class Model;	
	class Materials;
}

struct Vertex;

namespace Ge
{
    class ModelManager final : public InitializerAPI, public Manager
    {
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc * gdm);		
		void release();
		void updateStorage();
	public:
        ShapeBuffer * allocateBuffer(const char *path, bool normal_recalculate = false);        
        std::vector<ShapeBuffer*> allocateBuffers(const char* path, bool normal_recalculate = false);
		ShapeBuffer *allocateBuffer(float * pos, float * texCord, float * normal, unsigned int * indice, unsigned vertexSize, unsigned indiceSize);
		std::vector<ShapeBuffer*> allocateFBXBuffer(const char* path, bool normal_recalculate, std::vector<int> m_loadIdMesh);
		std::vector<ShapeBuffer*> allocateFBXBufferNoOptimize(const char* path, bool normal_recalculate, std::vector<int> m_loadIdMesh);
		void printModelInfo(const char *path);
        Model * createModel(ShapeBuffer *buffer, std::string nom = "Model");
        void destroyModel(Model *model);
        void destroyBuffer(ShapeBuffer *buffer);
        std::vector<Model*> & getModels();	
		std::unordered_map<Materials*, std::unordered_map<ShapeBuffer*,std::vector<Model*>>> & getInstancedModels();		
        void ComputationTangent(std::vector<Vertex> & vertices);
		ShapeBuffer* getDefferedQuad() const;
		ShapeBuffer* getFullScreenTriangle() const;
	private:
		friend class Model;		
		void buildInstancedModels(Model * target, Materials * mat);
		friend class MaterialManager;
		void clearInstancedMaterial(Materials * mat);
    private:		
		MemoryPool<Model> m_pool;
		MemoryPool<ShapeBufferBase> m_poolBuffer;
        std::vector<Model*> m_models;
		std::unordered_map<Materials*, std::unordered_map<ShapeBuffer*, std::vector<Model*>>> m_instanced;
        std::vector<ShapeBuffer *> m_shapeBuffers;        
		GraphicsDataMisc * m_gdm;
		ShapeBuffer* m_defferedQuad;
		ShapeBuffer* m_fullScreenTriangle;
    };
}

#endif //!__ENGINE_MODEL_MANAGER___