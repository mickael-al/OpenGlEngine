#ifndef __SOUND_BUFFER__
#define __SOUND_BUFFER__

#include <iostream>
#include "AL/al.h"
#include "AL/alc.h"

namespace Ge
{
	class SoundBuffer
	{
	public:
		SoundBuffer(uint64_t size, uint32_t frequency, ALenum format, int8_t* buffer,double time);
		ALuint getbufferID() const;
		float getTime() const;
		~SoundBuffer();
	private:
		ALuint m_bufferID;
		float m_time;
	};
}

#endif //!__SOUND_BUFFER__