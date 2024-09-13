#ifndef __AUDIO_SOURCE__
#define __AUDIO_SOURCE__

#include "GObject.hpp"

namespace Ge
{	
	class SoundBuffer;
	class AudioSource : public GObject
	{
	public:
		AudioSource(SoundBuffer* sb,std::string name = "AudioSource");
		void play();
		void pause();
		void stop();
		void setPitch(float pitch);
		void setGain(float gain);
		float getPitch() const;
		float getGain() const;
		float getTime() const;
		void setTime(float t);
		int SourceState();
		void setPosition(glm::vec3 pos) override;
		void setRotation(glm::quat rot) override;
		void setEulerAngles(glm::vec3 eul) override;
		void setVelocity(glm::vec3 vel);
		glm::vec3 getVelocity() const;
		void setLoop(bool state);
		bool getLoop() const;
		SoundBuffer * getSoundBuffer() const;

		void setRolloffFactor(float rolloffFactor);
		float getRolloffFactor() const;
		void setMaxDistance(float maxDistance);
		float getMaxDistance() const;
		void setRefDistance(float refDistance);
		float getRefDistance() const;
		void mapMemory() override;
		void onGUI() override;
		~AudioSource();
	private:
		SoundBuffer* m_sb = nullptr;
		unsigned int m_sourceID;
		float m_pitch = 1.0f;
		float m_gain = 1.0f;
		glm::vec3 m_velocity;
		bool m_loop = false;

		float orientation[6];
		float m_rolloffFactor = 1.0f;   // Rolloff factor
		float m_maxDistance = FLT_MAX;   // Max distance for attenuation
		float m_refDistance = 1.0f;		
	};
}
#endif //!__AUDIO_SOURCE__