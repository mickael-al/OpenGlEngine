#ifndef __ENGINE_DATA_MISC__
#define __ENGINE_DATA_MISC__

struct DataMisc
{
	unsigned int textureCount;
	unsigned int materialCount;
	unsigned int modelCount;
	unsigned int lightCount;
	bool recreateCommandBuffer;
};

#endif //!__ENGINE_DATA_MISC__