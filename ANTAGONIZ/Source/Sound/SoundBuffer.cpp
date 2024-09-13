#include "SoundBuffer.hpp"

namespace Ge
{
	SoundBuffer::SoundBuffer(uint64_t size, uint32_t frequency, ALenum format, int8_t* buffer,double time)
	{
		alGenBuffers(1, &m_bufferID);
		alBufferData(m_bufferID, format, buffer, size, frequency);		
		m_time = time;
	}

	ALuint SoundBuffer::getbufferID() const
	{
		return m_bufferID;
	}

	float SoundBuffer::getTime() const
	{
		return m_time;
	}

	SoundBuffer::~SoundBuffer()
	{
		alDeleteBuffers(1, &m_bufferID);
	}
}