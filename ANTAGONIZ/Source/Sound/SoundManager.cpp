#include "SoundManager.hpp"
#include "Debug.hpp"
#include "SoundTypeLoader.hpp"
#include "AL/al.h"
#include "AL/alc.h"
#include "SoundBuffer.hpp"
#include "AudioSource.hpp"

namespace Ge
{
	bool SoundManager::initialize()
	{
		m_pDevice = alcOpenDevice(nullptr);
		if (!m_pDevice)
		{
			Debug::Warn("Echec il y a aucun pheripherique audio!");
			return true;
		}
		m_pContext = alcCreateContext(m_pDevice, nullptr);
		alcMakeContextCurrent(m_pContext);
		if (!m_pContext)
		{
			Debug::Error("Echec de la creation du context audio!");
			return false;
		}
		Debug::INITSUCCESS("SoundManager");
		return true;
	}

	SoundBuffer * SoundManager::createBuffer(const char* filepath)
	{
		uint64_t size;
		uint32_t frequency;
		ALenum format;
		double time;
		int8_t* buffer = SoundTypeLoader::LoadWavFormat(filepath,&size,&frequency,&format,&time);
		
		if (buffer == nullptr)
		{
			Debug::Error("SoundBuffer nullptr");
			return nullptr;
		}

		SoundBuffer* sb = m_poolBuffer.newObject(size, frequency, format, buffer, time);

		delete[] buffer;
		buffer = nullptr;
		m_buffers.push_back(sb);
		return sb;
	}

	void SoundManager::releaseBuffer(SoundBuffer* sb)
	{
		auto it = std::find(m_buffers.begin(), m_buffers.end(), sb);
		if (it != m_buffers.end()) 
		{
			m_buffers.erase(it);
			m_poolBuffer.deleteObject(sb);			
		}
	}

	AudioSource* SoundManager::createSource(SoundBuffer* sb, std::string name)
	{		
		AudioSource* audio = m_poolAudio.newObject(sb, name);
		m_audios.push_back(audio);
		return audio;
	}

	void SoundManager::releaseSource(AudioSource* as)
	{
		auto it = std::find(m_audios.begin(), m_audios.end(), as);
		if (it != m_audios.end())
		{
			m_audios.erase(it);
			m_poolAudio.deleteObject(as);
		}
	}

	void SoundManager::setListenerPosition(glm::vec3 position)
	{
		alListener3f(AL_POSITION, position.x, position.y, position.z);
	}

	void SoundManager::setListenerVelocity(glm::vec3 velocity)
	{
		alListener3f(AL_POSITION, velocity.x, velocity.y, velocity.z);
	}

	void SoundManager::setListenerDirection(glm::vec3 direction)
	{
		m_orientation[0] = direction.x;
		m_orientation[1] = direction.y;
		m_orientation[2] = direction.z;
		m_orientation[3] = 0.0;
		m_orientation[4] = -1.0;
		m_orientation[5] = 0.0;
		alListenerfv(AL_ORIENTATION, m_orientation);
	}

	void SoundManager::setDistanceMode(int mode)
	{
		m_distanceMode = mode;
		alDistanceModel(mode);
	}

	int SoundManager::getDistanceMode() const
	{
		return m_distanceMode;
	}

	void SoundManager::release()
	{
		for (int i = 0; i < m_audios.size(); i++)
		{
			m_poolAudio.deleteObject(m_audios[i]);
		}
		m_audios.clear();
		for (int i = 0; i < m_buffers.size(); i++)
		{			
			m_poolBuffer.deleteObject(m_buffers[i]);
		}
		m_buffers.clear();
		alcMakeContextCurrent(NULL);
		alcDestroyContext(m_pContext);
		alcCloseDevice(m_pDevice);
		Debug::RELEASESUCCESS("SoundManager");
	}
}