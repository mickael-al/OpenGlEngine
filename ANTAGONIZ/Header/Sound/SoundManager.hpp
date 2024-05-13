#ifndef __SOUND_MANAGER__
#define __SOUND_MANAGER__

#include "Initializer.hpp"
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "MemoryManager.hpp"

struct ALCdevice;
struct ALCcontext;
namespace Ge
{	
	class SoundBuffer;
	class AudioSource;
	class SoundManager final : public Initializer
	{
	public:
		SoundBuffer * createBuffer(const char * filepath);
		void releaseBuffer(SoundBuffer * sb);		
		AudioSource* createSource(SoundBuffer* sb, std::string name = "AudioSource");
		void releaseSource(AudioSource* as);
		void setListenerPosition(glm::vec3 position);
		void setListenerVelocity(glm::vec3 velocity);
		void setListenerDirection(glm::vec3 direction);		
		void setDistanceMode(int mode);	
		int getDistanceMode() const;
	protected:
		friend class Engine;
		bool initialize();
		void release();
	private:
		MemoryPool<SoundBuffer> m_poolBuffer;
		MemoryPool<AudioSource> m_poolAudio;
		std::vector<SoundBuffer*> m_buffers;
		std::vector<AudioSource*> m_audios;
		ALCdevice* m_pDevice;
		ALCcontext* m_pContext;
		float m_orientation[6];
		int m_distanceMode = 0;//AL_NONE;
	};

};

#endif //!__SOUND_MANAGER__