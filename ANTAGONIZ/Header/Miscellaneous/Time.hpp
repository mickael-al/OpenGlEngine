#ifndef __ENGINE_TIME__
#define __ENGINE_TIME__

namespace Ge
{
	class Time final
	{
	private:
		friend class Engine;
		void startTime();
		void fixedUpdateTime();
		void updateTime();
		void release();
	public:
		Time();
		float getDeltaTime() const;
		float getFixedDeltaTime() const;
		float getTime() const;
		static float GetFixedDeltaTime();
		static float GetDeltaTime();
		static float GetTime();
	private:
		static Time* s_pInstance;
		double m_startTime;
		double m_currentTimeF;
		double m_lastTimeF;
		double m_currentTime;
		double m_lastTime;
		float m_time = 0.0f;
		float m_deltaTime = 0.0f;
		float m_fixedDeltaTime = 0.0f;
	};
}

#endif //!__ENGINE_TIME__