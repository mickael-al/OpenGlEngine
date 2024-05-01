#ifndef __ENGINE_MANAGER__
#define __ENGINE_MANAGER__

typedef unsigned int GLuint;

class Manager//SSBO Manager
{
protected:
	virtual void updateStorage() = 0;
	inline GLuint getSSBO() { return m_ssbo; }
protected:
	GLuint m_ssbo;//Shader storage Buffer Object
};

#endif //!__ENGINE_MANAGER__