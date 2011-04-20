/* The MIT License:

Copyright (c) 2008-2011 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Homepage: http://code.google.com/p/ting

/**
 * @file Timer.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @author Jose Luis Hidalgo <joseluis.hidalgo@gmail.com> - Mac OS X port
 * @brief Timer library.
 */

#pragma once

//#define M_ENABLE_TIMER_TRACE
#ifdef M_ENABLE_TIMER_TRACE
#define M_TIMER_TRACE(x) TRACE(<<"[Timer]" x)
#else
#define M_TIMER_TRACE(x)
#endif

#ifdef _MSC_VER //If Microsoft C++ compiler
#pragma warning(disable:4290) //WARNING: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#endif

//  ==System dependent headers inclusion==
#if defined(__WIN32__) || defined(WIN32)
#ifndef __WIN32__
#define __WIN32__
#endif

#include <windows.h>

#elif defined(__APPLE__)

#include<sys/time.h>

#elif defined(__linux__)

#include <ctime>

#else
#error "Unknown OS"
#endif
//~ ==System dependent headers inclusion==

#include <list>
#include <algorithm>

#include "debug.hpp" //debugging facilities
#include "types.hpp"
#include "Singleton.hpp"
#include "Thread.hpp"
#include "math.hpp"
#include "Signal.hpp"



namespace ting{



//forward declarations
class Timer;



//function prototypes
inline ting::u32 GetTicks();



typedef ting::u16 TTicks; //thicks type, should be unsigned. TODO: change to ting::u32 after testing



class TimerLib : public Singleton<TimerLib>{
	friend class ting::Timer;

	class TimerThread : public ting::Thread{
	public:
		ting::Inited<bool, false> quitFlag;

		ting::Mutex mutex;
		ting::Semaphore sema;

		typedef std::map<Timer*> T_TimerList;
		typedef T_TimerList::iterator T_TimerIter;
		T_TimerList timers;

		ting::Inited<ting::u64, 0> ticks;

		ting::Inited<bool, false> incTicks;

		TimerThread(){
			ASSERT(!this->quitFlag)
		}

		~TimerThread(){
			//at the time of TimerLib destroying there should be no active timers
			ASSERT(this->timers.size() == 0)
		}

		inline void AddTimer(Timer* timer, u32 timeout);

		inline Timer::EState RemoveTimer(Timer* timer);

		inline void SetQuitFlagAndSignalSemaphore(){
			this->quitFlag = true;
			this->sema.Signal();
		}

		//override (inline is just to make possible method definition in hpp)
		inline void Run();

	private:
//		inline void UpdateTimer(Timer* timer, u32 newTimeout);

	} thread;

	inline void AddTimer(Timer* timer, u32 timeout){
		this->thread.AddTimer(timer, timeout);
	}

	inline bool RemoveTimer(Timer* timer){
		return this->thread.RemoveTimer(timer);
	}

public:
	TimerLib(){
		this->thread.Start();
	}

	~TimerLib(){
		this->thread.SetQuitFlagAndSignalSemaphore();
		this->thread.Join();
	}
};



class Timer{
	friend class TimerLib::TimerThread;

public:
	enum EState{
		NOT_STARTED,
		RUNNING,
		EXPIRED
	};

private:
	ting::TimerLib::TimerThread::T_TimerIter i;//if satte is RUNNING, this is the iterator into the map of timers

	ting::Inited<EState, NOT_STARTED> state;
public:

	ting::Signal1<Timer&> expired;

	inline Timer(){
		ASSERT(this->state == NOT_STARTED)
	}

	virtual ~Timer(){
		ASSERT(TimerLib::IsCreated())
		this->Stop();

		ASSERT(
				this->state == NOT_STARTED ||
				this->state == EXPIRED
			)
	}

	inline void Start(ting::u32 millisec){
		ASSERT_INFO(TimerLib::IsCreated(), "Timer library is not initialized, you need to create TimerLib singletone object first")

		this->Stop();//make sure the timer is not running already
		TimerLib::Inst().AddTimer(this, millisec);
	}

	/**
	 * @brief Stop the timer.
	 * Stops the timer if it was started before. In case it was not started
	 * or it has already expired this method does nothing.
	 * @return the state in which the timer was at the moment of stopping. If returned state is
	 *         RUNNING then the timer was started before the Stop() method was called, and, as a result
	 *         of the method call, it was stopped before expiring and
	 *         the exired signal was not emitted. If the returned state is EXPIRED
	 *         then the timer has expired already at the moment the Stop() method was called
	 *         and the expired signal has been emitted before stopping. If the returned state is NOT_STARTED
	 *         then the timer has not been started yet at the moment the Stop() method was
	 *         called and the expired signal was not emitted, of course.
	 */
	inline EState Stop(){
		ASSERT(TimerLib::IsCreated())
		return TimerLib::Inst().RemoveTimer(this);
	}
};



//methods

//inline void TimerLib::TimerThread::UpdateTimer(Timer* timer, u32 newTimeout){
//	ting::u32 curTicks = ting::GetTicks();
//
//	timer->endTime = curTicks + newTimeout;
//
//	if(timer->endTime < curTicks){
//		timer->warp = true;
//	}else{
//		timer->warp = false;
//	}
//}



inline Timer::EState TimerLib::TimerThread::RemoveTimer(Timer* timer){
	ASSERT(timer)
	ting::Mutex::Guard mutexGuard(this->mutex);

	if(timer->state != Timer::RUNNING)
		return timer->state;

	//if isStarted flag is set then the timer will be stopped now, so
	//change the flag
	timer->state = Timer::NOT_STARTED;

	ASSERT(timer->i != this->timers.end())

	this->timers.erase(timer->i);
	timer->i = this->timers.end();

	//was in RUNNING state
	return Timer::RUNNING;
}



inline void TimerLib::TimerThread::AddTimer(Timer* timer, u32 timeout){
	ASSERT(timer)
	ting::Mutex::Guard mutexGuard(this->mutex);

	if(timer->state == Timer::RUNNING)
		throw ting::Exc("TimerLib::TimerThread::AddTimer(): timer is already reunning!");

	timer->state = Timer::RUNNING;

	ting::u64 stopTicks =

	this->UpdateTimer(timer, timeout);

	this->timers.push_back(timer);
	this->sema.Signal();
}



//override
inline void TimerLib::TimerThread::Run(){
	M_TIMER_TRACE(<< "TimerLib::TimerThread::Run(): enter" << std::endl)
	//init warp flag
	if(ting::GetTicks() < ting::u32(-1) / 2){
		this->warpFlag = false;
	}else{
		this->warpFlag = true;
	}

	M_TIMER_TRACE(<< "TimerLib::TimerThread::Run(): creating mutex guard" << std::endl)
	ting::Mutex::Guard mutexGuard(this->mutex);
	M_TIMER_TRACE(<< "TimerLib::TimerThread::Run(): entering while()" << std::endl)
	while(!this->quitFlag){
		//check warp
		ting::u32 ticks = ting::GetTicks();
		M_TIMER_TRACE(<<"TimerThread: ticks = " << ticks << std::endl)

		if(ticks < ting::u32(-1) / 2){
			if(this->warpFlag){
				//Warp detected.
				//clear all warp flags, remove timers which was not warped
				for(T_TimerIter i = this->timers.begin(); i != this->timers.end();){
					//if the timer was not warped and we are warping
					//then this timer is surely expired, need to remove it and
					//notify client calling OnExpire().
					if(!(*i)->warp){
						u32 newTimeout = (*i)->OnExpire();
						if(newTimeout == 0){
							(*i)->isStarted = false;
							i = this->timers.erase(i);
							continue;
						}else{
							this->UpdateTimer(*i, newTimeout);
						}
					}else{
						(*i)->warp = false;
					}
					++i;
				}
			}
			this->warpFlag = false;
		}else{
			this->warpFlag = true;
		}

		//notify expired timers
		M_TIMER_TRACE(<<"TimerThread: search for expired timers, size = " << this->timers.size() << std::endl)
		for(T_TimerIter i = this->timers.begin(); i != this->timers.end();){
			if(!(*i)->warp){
				M_TIMER_TRACE(<<"TimerThread: warp is not set, endTime = "<< (*i)->endTime << std::endl)
				if((*i)->endTime <= ticks){
					u32 newTimeout = (*i)->OnExpire();
					if(newTimeout == 0){
						(*i)->isStarted = false;
						i = this->timers.erase(i);
						continue;
					}else{//set timer again
						this->UpdateTimer(*i, newTimeout);
					}
				}
			}else{
				M_TIMER_TRACE(<<"TimerThread: warp is set" << std::endl)
			}
			++i;
		}

		if(this->timers.size() == 0){
			this->mutex.Unlock();
			M_TIMER_TRACE(<<"TimerThread: waiting forever" << std::endl)
			this->sema.Wait();
			M_TIMER_TRACE(<<"TimerThread: signalled" << std::endl)
			this->mutex.Lock();
			continue;
		}

		//calculate number of milliseconds to wait
		ting::u32 minEndTime = ting::u32(-1);
		for(T_TimerIter i = this->timers.begin(); i != this->timers.end(); ++i){
			if(!(*i)->warp){
				if((*i)->endTime < minEndTime){
					minEndTime = (*i)->endTime;
				}
			}
		}

		unsigned millis = minEndTime - ticks;

		//make sure we will update warpFlag at least 4 times
		//per ticks cycle (ticks cycle = 0xffffffffff ticks)
		millis = (std::min)(millis, ting::u32(-1) / 4);//NOTE: enclose (std::min) into parentheses in order to avoid compilation errors when using MSVC compiler, because windows.h defines min and max macros.
		ASSERT(millis != 0)

		this->mutex.Unlock();

		M_TIMER_TRACE(<<"TimerThread: waiting for "<<millis<< " ms" << std::endl)
		this->sema.Wait(millis);
		M_TIMER_TRACE(<<"TimerThread: signalled" << std::endl)
		//It does not matter signalled or timed out

		this->mutex.Lock();
	}//~while

	M_TIMER_TRACE(<< "TimerLib::TimerThread::Run(): exit" << std::endl)
}//~Run()



/**
 * @brief Get constantly increasing millisecond ticks.
 * It is not guaranteed that the ticks counting started at the system start.
 * @return constantly increasing millisecond ticks.
 */
inline ting::u32 GetTicks(){
#ifdef __WIN32__
	static LARGE_INTEGER perfCounterFreq = {{0, 0}};
	if(perfCounterFreq.QuadPart == 0){
		if(QueryPerformanceFrequency(&perfCounterFreq) == FALSE){
			//looks like the system does not support high resolution tick counter
			return GetTickCount();
		}
	}
	LARGE_INTEGER ticks;
	if(QueryPerformanceCounter(&ticks) == FALSE){
		return GetTickCount();
	}

	return ting::u32((ticks.QuadPart * 1000) / perfCounterFreq.QuadPart);
#elif defined(__APPLE__)
	//Mac os X doesn't support clock_gettime
	timeval t;
	gettimeofday(&t, 0);
	return ting::u32(t.tv_sec * 1000 + (t.tv_usec / 1000));
#elif defined(__linux__)
	timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
		throw ting::Exc("GetTicks(): clock_gettime() returned error");

	return u32(u32(ts.tv_sec) * 1000 + u32(ts.tv_nsec / 1000000));
#else
#error "Unsupported OS"
#endif
}



}//~namespace
