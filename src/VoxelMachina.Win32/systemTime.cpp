#include "pch.h"
#include "systemTime.h"
#include "assert.h"

double SystemTime::sm_CpuTickDelta = 0.0;

void SystemTime::Initialize(void)
{
	LARGE_INTEGER frequency;
	ASSERT(TRUE == QueryPerformanceFrequency(&frequency), "Unable to query performance counter frequency");
	sm_CpuTickDelta = 1.0 / static_cast<double>(frequency.QuadPart);
}

// Query the current value of the performance counter
int64_t SystemTime::GetCurrentTick(void)
{
	LARGE_INTEGER currentTick;
	ASSERT(TRUE == QueryPerformanceCounter(&currentTick), "Unable to query performance counter value");
	return static_cast<int64_t>(currentTick.QuadPart);
}

void SystemTime::BusyLoopSleep(float SleepTime)
{
	int64_t finalTick = (int64_t)((double)SleepTime / sm_CpuTickDelta) + GetCurrentTick();
	while (GetCurrentTick() < finalTick);
}