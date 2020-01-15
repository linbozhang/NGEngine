#include "stdafx.h"
#include "GameTimer.h"
GameTimer::GameTimer(): mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0), mPausedTime(0), mStopTime(0), mPrevTime(0),
                        mCurrTime(0),
                        mStopped(false)
{
    __int64 countsPerSec = 0;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(countsPerSec));
    mSecondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

float GameTimer::TotalTime() const
{
	 if(mStopped)
	 {
         return (float)(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);	 	
	 }else
	 {
		 return (float)(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	 }
}

float GameTimer::DeltaTime() const
{
	return (float)mDeltaTime;
}
void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
	if(mStopped)
	{
		mPausedTime += (startTime - mStopTime);
		mPrevTime = startTime;
		mStopTime = 0;
		mStopped = false;
	}
}

void GameTimer::Stop()
{
	if(!mStopped)
	{
		mStopped = true;
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mStopTime = currTime;		
	}
}
void GameTimer::Tick()
{
	if(mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;
	mPrevTime = mCurrTime;
	if(mDeltaTime<0.0)
	{
		mDeltaTime = 0.0;
	}
}



