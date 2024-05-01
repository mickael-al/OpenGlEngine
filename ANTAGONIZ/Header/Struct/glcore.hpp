#ifndef __ENGINE_OPENG_GL_CORE__
#define __ENGINE_OPENG_GL_CORE__

#include <cstdint>
#define GLEW_STATIC 1
//#include <GL/glew.h>
#include "glew/include/GL/glew.h"
#ifdef _WIN32
//#include <GL/wglew.h>
#include "glew/include/GL/wglew.h"
#elif __unix__

#endif

enum BufferType
{
	VBO,
	IBO,
	MAX
};

#endif //!__ENGINE_OPENG_GL_CORE__
