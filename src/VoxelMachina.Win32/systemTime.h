#pragma once

namespace SystemTime
{
	extern double sm_CpuTickDelta;

	// Query the performance counter frequency
	void Initialize(void);

	// Query the current value of the performance counter
	int64_t GetCurrentTick(void);

	void BusyLoopSleep(float SleepTime);

	inline double TicksToSeconds(int64_t TickCount)
	{
		return TickCount * sm_CpuTickDelta;
	}

	inline double TicksToMillisecs(int64_t TickCount)
	{
		return TickCount * sm_CpuTickDelta * 1000.0;
	}

	static inline double TimeBetweenTicks(int64_t tick1, int64_t tick2)
	{
		return TicksToSeconds(tick2 - tick1);
	}

	// The amount of time that elapses between ticks of the performance counter
}