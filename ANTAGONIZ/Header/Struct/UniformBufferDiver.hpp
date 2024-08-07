#ifndef __ENGINE_UNIFORM_BUFFER_DIVER__
#define __ENGINE_UNIFORM_BUFFER_DIVER__

struct UniformBufferDiver
{
    int maxLight;
    float u_time;
    float gamma;
    float ambiant;
	float fov;
	bool ortho;
};

#endif //!__ENGINE_UNIFORM_BUFFER_DIVER__