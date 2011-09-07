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
 * @file Thread.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @author Jose Luis Hidalgo <joseluis.hidalgo@gmail.com> - Mac OS X port
 * @brief Multithreading library.
 */

#pragma once

#include <cstring>
#include <sstream>

#include "debug.hpp"
#include "Ptr.hpp"
#include "types.hpp"
#include "Exc.hpp"
#include "WaitSet.hpp"


//=========
//= WIN32 =
//=========
#if defined(WIN32)

//if _WINSOCKAPI_ macro is not defined then it means that the winsock header file
//has not been included. Here we temporarily define the macro in order to prevent
//inclusion of winsock.h from within the windows.h. Because it may later conflict with
//winsock2.h if it is included later.
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#include <windows.h>
#undef _WINSOCKAPI_
#else
#include <windows.h>
#endif

#include <process.h>



//===========
//= Symbian =
//===========
#elif defined(__SYMBIAN32__)
#include <string.h>
#include <e32std.h>
#include <hal.h>



//========================================
//= Linux, Mac OS (Apple), Solaris (Sun) =
//========================================
#elif defined(__linux__) || defined(__APPLE__) || defined(sun) || defined(__sun)

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <ctime>

//if we have Solaris
#if defined(sun) || defined(__sun)
#include <sched.h>	//	for sched_yield();
#endif

#if defined(__linux__)
#include <sys/eventfd.h>
#endif



#else
#error "Unsupported OS"
#endif



//if Microsoft MSVC compiler,
//then disable warning about throw specification is ignored.
#ifdef _MSC_VER
#pragma warning(push) //push warnings state
#pragma warning( disable : 4290)
#endif



//#define M_ENABLE_MUTEX_TRACE
#ifdef M_ENABLE_MUTEX_TRACE
#define M_MUTEX_TRACE(x) TRACE(<<"[MUTEX] ") TRACE(x)
#else
#define M_MUTEX_TRACE(x)
#endif


//#define M_ENABLE_QUEUE_TRACE
#ifdef M_ENABLE_QUEUE_TRACE
#define M_QUEUE_TRACE(x) TRACE(<<"[QUEUE] ") TRACE(x)
#else
#define M_QUEUE_TRACE(x)
#endif


namespace ting{

//forward declarations
class CondVar;
class Queue;
class Thread;
class QuitMessage;

/**
 * @brief Mutex object class
 * Mutex stands for "Mutual execution".
 */
class Mutex{
	friend class CondVar;

	//system dependent handle
#ifdef WIN32
	CRITICAL_SECTION m;
#elif defined(__SYMBIAN32__)
	RCriticalSection m;
#elif defined(__linux__) || defined(__APPLE__)
	pthread_mutex_t m;
#else
#error "unknown system"
#endif

	//forbid copying
	Mutex(const Mutex& ){
		ASSERT(false)
	}
	Mutex& operator=(const Mutex& ){
		return *this;
	}

public:
	/**
	 * @brief Creates initially unlocked mutex.
	 */
	Mutex(){
		M_MUTEX_TRACE(<< "Mutex::Mutex(): invoked " << this << std::endl)
#ifdef WIN32
		InitializeCriticalSection(&this->m);
#elif defined(__SYMBIAN32__)
		if(this->m.CreateLocal() != KErrNone){
			throw ting::Exc("Mutex::Mutex(): failed creating mutex (CreateLocal() failed)");
		}
#elif defined(__linux__) || defined(__APPLE__)
		if(pthread_mutex_init(&this->m, NULL) != 0){
			throw ting::Exc("Mutex::Mutex(): failed creating mutex (pthread_mutex_init() failed)");
		}
#else
#error "unknown system"
#endif
	}



	~Mutex(){
		M_MUTEX_TRACE(<< "Mutex::~Mutex(): invoked " << this << std::endl)
#ifdef WIN32
		DeleteCriticalSection(&this->m);
#elif defined(__SYMBIAN32__)
		this->m.Close();
#elif defined(__linux__) || defined(__APPLE__)
		int ret = pthread_mutex_destroy(&this->m);
		if(ret != 0){
			std::stringstream ss;
			ss << "Mutex::~Mutex(): pthread_mutex_destroy() failed"
					<< " error code = " << ret << ": " << strerror(ret) << ".";
			if(ret == EBUSY){
				ss << " You are trying to destroy locked mutex.";
			}
			ASSERT_INFO_ALWAYS(false, ss.str())
		}
#else
#error "unknown system"
#endif
	}



	/**
	 * @brief Acquire mutex lock.
	 * If one thread acquired the mutex lock then all other threads
	 * attempting to acquire the lock on the same mutex will wait until the
	 * mutex lock will be released with Mutex::Unlock().
	 */
	void Lock(){
		M_MUTEX_TRACE(<< "Mutex::Lock(): invoked " << this << std::endl)
#ifdef WIN32
		EnterCriticalSection(&this->m);
#elif defined(__SYMBIAN32__)
		this->m.Wait();
#elif defined(__linux__) || defined(__APPLE__)
		pthread_mutex_lock(&this->m);
#else
#error "unknown system"
#endif
	}



	/**
	 * @brief Release mutex lock.
	 */
	void Unlock(){
		M_MUTEX_TRACE(<< "Mutex::Unlock(): invoked " << this << std::endl)
#ifdef WIN32
		LeaveCriticalSection(&this->m);
#elif defined(__SYMBIAN32__)
		this->m.Signal();
#elif defined(__linux__) || defined(__APPLE__)
		pthread_mutex_unlock(&this->m);
#else
#error "unknown system"
#endif
	}



	/**
	 * @brief Helper class which automatically Locks the given mutex.
	 * This helper class automatically locks the given mutex in the constructor and
	 * unlocks the mutex in destructor. This class is useful if the code between
	 * mutex lock/unlock may return or throw an exception,
	 * then the mutex be automaticlly unlocked in such case.
	 */
	class Guard{
		Mutex *mut;

		//forbid copying
		Guard(const Guard& ){
			ASSERT(false)
		}
		Guard& operator=(const Guard& ){
			return *this;
		}
	public:
		Guard(Mutex &m):
				mut(&m)
		{
			this->mut->Lock();
		}
		~Guard(){
			this->mut->Unlock();
		}
	};//~class Guard
};//~class Mutex



/**
 * @brief Semaphore class.
 * The semaphore is actually an unsigned integer value which can be incremented
 * (by Semaphore::Signal()) or decremented (by Semaphore::Wait()). If the value
 * is 0 then any try to decrement it will result in execution blocking of the current thread
 * until the value is incremented so the thread will be able to
 * decrement it. If there are several threads waiting for semaphore decrement and
 * some other thread increments it then only one of the hanging threads will be
 * resumed, other threads will remain waiting for next increment.
 */
class Semaphore{
	//system dependent handle
#ifdef WIN32
	HANDLE s;
#elif defined(__SYMBIAN32__)
	RSemaphore s;
#elif defined(__APPLE__)
	//TODO: consider using the MPCreateSemaphore
	sem_t *s;
#elif defined(__linux__)
	sem_t s;
#else
#error "unknown system"
#endif

	//forbid copying
	Semaphore(const Semaphore& );
	Semaphore& operator=(const Semaphore& );
    
public:

	/**
	 * @brief Create the semaphore with given initial value.
	 */
	Semaphore(unsigned initialValue = 0){
#ifdef WIN32
		if( (this->s = CreateSemaphore(NULL, initialValue, 0xffffff, NULL)) == NULL)
#elif defined(__SYMBIAN32__)
		if(this->s.CreateLocal(initialValue) != KErrNone)
#elif defined(__APPLE__)
		// Darwin/BSD/... semaphores are named semaphores, we need to create a 
		// different name for new semaphores.
		char name[256];
		// this n_name is shared among all semaphores, maybe it will be worth protect it
		// by a mutex or a CAS operation;
		static unsigned int n_name = 0;
		unsigned char p = 0;
		// fill the name
		for(unsigned char n = ++n_name, p = 0; n > 0;){
			unsigned char idx = n%('Z'-'A'+1);
			name[++p] = 'A'+idx;
			n -= idx;
		}
		// end it with null and create the semaphore
		name[p] = '\0';
		this->s = sem_open(name, O_CREAT, SEM_VALUE_MAX, initialValue);
		if (this->s == SEM_FAILED)
#elif defined(__linux__)
		if(sem_init(&this->s, 0, initialValue) < 0)
#else
#error "unknown system"
#endif
		{
			LOG(<< "Semaphore::Semaphore(): failed" << std::endl)
			throw ting::Exc("Semaphore::Semaphore(): creating semaphore failed");
		}
	}



	~Semaphore(){
#ifdef WIN32
		CloseHandle(this->s);
#elif defined(__SYMBIAN32__)
		this->s.Close();
#elif defined(__APPLE__)
		sem_close(this->s);
#elif defined(__linux__)
		sem_destroy(&this->s);
#else
#error "unknown system"
#endif
	}

	
	
	/**
	 * @brief Wait on semaphore.
	 * Decrements semaphore value. If current value is 0 then this method will wait
	 * until some other thread signals the semaphore (i.e. increments the value)
	 * by calling Semaphore::Signal() on that semaphore.
	 */
	void Wait(){
#ifdef WIN32
		switch(WaitForSingleObject(this->s, DWORD(INFINITE))){
			case WAIT_OBJECT_0:
//				LOG(<<"Semaphore::Wait(): exit"<<std::endl)
				break;
			case WAIT_TIMEOUT:
				ASSERT(false)
				break;
			default:
				throw ting::Exc("Semaphore::Wait(): wait failed");
				break;
		}
#elif defined(__SYMBIAN32__)
		this->s.Wait();
#elif defined(__APPLE__)
		int retVal = 0;

		do{
			retVal = sem_wait(this->s);
		}while(retVal == -1 && errno == EINTR);

		if(retVal < 0){
			throw ting::Exc("Semaphore::Wait(): wait failed");
		}
#elif defined(__linux__)
		int retVal;
		do{
			retVal = sem_wait(&this->s);
		}while(retVal == -1 && errno == EINTR);
		if(retVal < 0){
			throw ting::Exc("Semaphore::Wait(): wait failed");
		}
#else
#error "unknown system"
#endif
	}
	


	/**
	 * @brief Wait on semaphore with timeout.
	 * Decrements semaphore value. If current value is 0 then this method will wait
	 * until some other thread signals the semaphore (i.e. increments the value)
	 * by calling Semaphore::Signal() on that semaphore.
	 * @param timeoutMillis - waiting timeout.
	 *                        If timeoutMillis is 0 (the default value) then this
	 *                        method will try to decrement the semaphore value and exit immediately.
	 * @return returns true if the semaphore value was decremented.
	 * @return returns false if the timeout was hit.
	 */
	bool Wait(ting::u32 timeoutMillis){
#ifdef WIN32
		STATIC_ASSERT(INFINITE == 0xffffffff)
		switch(WaitForSingleObject(this->s, DWORD(timeoutMillis == INFINITE ? INFINITE - 1 : timeoutMillis))){
			case WAIT_OBJECT_0:
				return true;
			case WAIT_TIMEOUT:
				return false;
			default:
				throw ting::Exc("Semaphore::Wait(u32): wait failed");
				break;
		}
#elif defined(__SYMBIAN32__)
		throw ting::Exc("Semaphore::Wait(): wait with timeout is not implemented on Symbian, TODO: implement");
#elif defined(__APPLE__)
		int retVal = 0;

		// simulate the behavior of wait
		do{
			retVal = sem_trywait(this->s);
			if(retVal == 0){
				break; // OK leave the loop
			}else{
				if(errno == EAGAIN){ // the semaphore was blocked
					struct timespec amount;
					struct timespec result;
					int resultsleep;
					amount.tv_sec = timeoutMillis / 1000;
					amount.tv_nsec = (timeoutMillis % 1000) * 1000000;
					resultsleep = nanosleep(&amount, &result);
					// update timeoutMillis based on the output of the sleep call
					// if nanosleep() returns -1 the sleep was interrupted and result
					// struct is updated with the remaining un-slept time.
					if(resultsleep == 0){
						timeoutMillis = 0;
					}else{
						timeoutMillis = result.tv_sec * 1000 + result.tv_nsec / 1000000;
					}
				}else if(errno != EINTR){
					throw ting::Exc("Semaphore::Wait(): wait failed");
				}
			}
		}while(timeoutMillis > 0);

		// no time left means we reached the timeout
		if(timeoutMillis == 0){
			return false;
		}
#elif defined(__linux__)
		//if timeoutMillis is 0 then use sem_trywait() to avoid unnecessary time calculation for sem_timedwait()
		if(timeoutMillis == 0){
			if(sem_trywait(&this->s) == -1){
				if(errno == EAGAIN){
					return false;
				}else{
					throw ting::Exc("Semaphore::Wait(u32): error: sem_trywait() failed");
				}
			}
		}else{
			timespec ts;

			if(clock_gettime(CLOCK_REALTIME, &ts) == -1)
				throw ting::Exc("Semaphore::Wait(): clock_gettime() returned error");

			ts.tv_sec += timeoutMillis / 1000;
			ts.tv_nsec += (timeoutMillis % 1000) * 1000 * 1000;
			ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
			ts.tv_nsec = ts.tv_nsec % (1000 * 1000 * 1000);

			if(sem_timedwait(&this->s, &ts) == -1){
				if(errno == ETIMEDOUT){
					return false;
				}else{
					throw ting::Exc("Semaphore::Wait(u32): error: sem_timedwait() failed");
				}
			}
		}
#else
#error "unknown system"
#endif
		return true;
	}



	/**
	 * @brief Signal the semaphore.
	 * Increments the semaphore value.
	 */
	inline void Signal(){
//		TRACE(<< "Semaphore::Signal(): invoked" << std::endl)
#ifdef WIN32
		if( ReleaseSemaphore(this->s, 1, NULL) == 0 ){
			throw ting::Exc("Semaphore::Post(): releasing semaphore failed");
		}
#elif defined(__SYMBIAN32__)
		this->s.Signal();
#elif defined(__APPLE__)
		if(sem_post(this->s) < 0){
			throw ting::Exc("Semaphore::Post(): releasing semaphore failed");
		}
#elif defined(__linux__)
		if(sem_post(&this->s) < 0){
			throw ting::Exc("Semaphore::Post(): releasing semaphore failed");
		}
#else
#error "unknown system"
#endif
	}
};//~class Semaphore



class CondVar{
#if defined(WIN32) || defined(__SYMBIAN32__)
	Mutex cvMutex;
	Semaphore semWait;
	Semaphore semDone;
	unsigned numWaiters;
	unsigned numSignals;
#elif defined(__linux__) || defined(__APPLE__)
	//A pointer to store system dependent handle
	pthread_cond_t cond;
#else
#error "unknown system"
#endif

	//forbid copying
	CondVar(const CondVar& ){
		ASSERT(false)
	}
	CondVar& operator=(const CondVar& ){
		return *this;
	}
	
public:

	CondVar(){
#if defined(WIN32) || defined(__SYMBIAN32__)
		this->numWaiters = 0;
		this->numSignals = 0;
#elif defined(__linux__) || defined(__APPLE__)
		pthread_cond_init(&this->cond, NULL);
#else
#error "unknown system"
#endif
	}

	~CondVar(){
#if defined(WIN32) || defined(__SYMBIAN32__)
		//do nothing
#elif defined(__linux__) || defined(__APPLE__)
	pthread_cond_destroy(&this->cond);
#else
#error "unknown system"
#endif
	}

	void Wait(Mutex& mutex){
#if defined(WIN32) || defined(__SYMBIAN32__)
		this->cvMutex.Lock();
		++this->numWaiters;
		this->cvMutex.Unlock();

		mutex.Unlock();

		this->semWait.Wait();

		this->cvMutex.Lock();
		if(this->numSignals > 0){
			this->semDone.Signal();
			--this->numSignals;
		}
		--this->numWaiters;
		this->cvMutex.Unlock();

		mutex.Lock();
#elif defined(__linux__) || defined(__APPLE__)
		pthread_cond_wait(&this->cond, &mutex.m);
#else
#error "unknown system"
#endif
	}

	void Notify(){
#if defined(WIN32) || defined(__SYMBIAN32__)
		this->cvMutex.Lock();

		if(this->numWaiters > this->numSignals){
			++this->numSignals;
			this->semWait.Signal();
			this->cvMutex.Unlock();
			this->semDone.Wait();
		}else{
			this->cvMutex.Unlock();
		}
#elif defined(__linux__) || defined(__APPLE__)
		pthread_cond_signal(&this->cond);
#else
#error "unknown system"
#endif
	}
};



/**
 * @brief Message abstract class.
 * The messages are sent to message queues (see ting::Queue). One message instance cannot be sent to
 * two or more message queues, but only to a single queue. When sent, the message is
 * further owned by the queue (note the usage of ting::Ptr auto-pointers).
 */
class Message{
	friend class Queue;

	Message *next;//pointer to the next message in a single-linked list

protected:
	Message() :
			next(0)
	{}

public:
	virtual ~Message(){}

	/**
	 * @brief message handler function.
	 * This virtual method is called to handle the message. When deriving from ting::Message,
	 * override this method to define the message handler procedure.
	 */
	virtual void Handle() = 0;
};



/**
 * @brief Message queue.
 * Message queue is used for communication of separate threads by
 * means of sending messages to each other. Thus, when one thread sends a message to another one,
 * it asks that another thread to execute some code portion - handler code of the message.
 * Each Thread object already contains its own queue object (Thread::queue), but one is free to
 * create his/her own Queue objects when they are needed.
 * NOTE: Queue implements Waitable interface which means that it can be used in conjunction
 * with ting::WaitSet API. But, note, that the implementation of the Waitable is that it
 * shall only be used to wait for READ. If you are trying to wait for WRITE the behavior will be
 * undefined.
 */
class Queue : public Waitable{
	Semaphore sem;

	Mutex mut;

	Message *first,
			*last;

#if defined(WIN32)
	//use additional semaphore to implement Waitable on Windows
	HANDLE eventForWaitable;
#elif defined(__linux__)
	//use eventfd()
	int eventFD;
#elif defined(__APPLE__)
	//use pipe to implement Waitable in *nix systems
	int pipeEnds[2];
#else
#error "Unsupported OS"
#endif

	//forbid copying
	Queue(const Queue&);
	Queue& operator=(const Queue&);

public:
	/**
	 * @brief Constructor, creates empty message queue.
	 */
	Queue() :
			first(0),
			last(0)
	{
		//can write will always be set because it is always possible to post a message to the queue
		this->SetCanWriteFlag();

#if defined(WIN32)
		this->eventForWaitable = CreateEvent(
				NULL, //security attributes
				TRUE, //manual-reset
				FALSE, //not signalled initially
				NULL //no name
			);
		if(this->eventForWaitable == NULL){
			throw ting::Exc("Queue::Queue(): could not create event (Win32) for implementing Waitable");
		}
#elif defined(__linux__)
		this->eventFD = eventfd(0, EFD_NONBLOCK);
		if(this->eventFD < 0){
			std::stringstream ss;
			ss << "Queue::Queue(): could not create eventfd (linux) for implementing Waitable,"
					<< " error code = " << errno << ": " << strerror(errno);
			throw ting::Exc(ss.str().c_str());
		}
#elif defined(__APPLE__)
		if(::pipe(&this->pipeEnds[0]) < 0){
			std::stringstream ss;
			ss << "Queue::Queue(): could not create pipe (*nix) for implementing Waitable,"
					<< " error code = " << errno << ": " << strerror(errno);
			throw ting::Exc(ss.str().c_str());
		}
#else
#error "Unsupported OS"
#endif
	}



	/**
	 * @brief Destructor.
	 * When called, it also destroys all messages on the queue.
	 */
	~Queue(){
		//destroy messages which are currently on the queue
		{
			Mutex::Guard mutexGuard(this->mut);
			Message *msg = this->first;
			Message	*nextMsg;
			while(msg){
				nextMsg = msg->next;
				//use Ptr to kill messages instead of "delete msg;" because
				//the messages are passed to PushMessage() as Ptr, and thus, it is better
				//to use Ptr to delete them.
				{Ptr<Message> killer(msg);}

				msg = nextMsg;
			}
		}
#if defined(WIN32)
		CloseHandle(this->eventForWaitable);
#elif defined(__linux__)
		close(this->eventFD);
#elif defined(__APPLE__)
		close(this->pipeEnds[0]);
		close(this->pipeEnds[1]);
#else
#error "Unsupported OS"
#endif
	}



	/**
	 * @brief Pushes a new message to the queue.
	 * @param msg - the message to push into the queue.
	 */
	void PushMessage(Ptr<Message> msg){
		ASSERT(msg.IsValid())
		Mutex::Guard mutexGuard(this->mut);
		if(this->first){
			ASSERT(this->last && this->last->next == 0)
			this->last = this->last->next = msg.Extract();
			ASSERT(this->last->next == 0)
		}else{
			ASSERT(msg.IsValid())
			this->last = this->first = msg.Extract();

			//Set CanRead flag.
			//NOTE: in linux imlementation with epoll(), the CanRead
			//flag will also be set in WaitSet::Wait() method.
			//NOTE: set CanRead flag before event notification/pipe write, because
			//if do it after then some other thread which was waiting on the WaitSet
			//may read the CanRead flag while it was not set yet.
			ASSERT(!this->CanRead())
			this->SetCanReadFlag();
			
#if defined(WIN32)
			if(SetEvent(this->eventForWaitable) == 0){
				throw ting::Exc("Queue::PushMessage(): setting event for Waitable failed");
			}
#elif defined(__linux__)
			if(eventfd_write(this->eventFD, 1) < 0){
				throw ting::Exc("Queue::PushMessage(): eventfd_write() failed");
			}
#elif defined(__APPLE__)
			{
				u8 oneByteBuf[1];
				write(this->pipeEnds[1], oneByteBuf, 1);
			}
#else
#error "Unsupported OS"
#endif
		}

		ASSERT(this->CanRead())
		//NOTE: must do signalling while mutex is locked!!!
		this->sem.Signal();
	}



	/**
	 * @brief Get message from queue, does not block if no messages queued.
	 * This method gets a message from message queue. If there are no messages on the queue
	 * it will return invalid auto pointer.
	 * @return auto-pointer to Message instance.
	 * @return invalid auto-pointer if there are no messares in the queue.
	 */
	Ptr<Message> PeekMsg(){
		Mutex::Guard mutexGuard(this->mut);
		if(this->first){
			ASSERT(this->CanRead())
			//NOTE: Decrement semaphore value, because we take one message from queue.
			//      The semaphore value should be > 0 here, so there will be no hang
			//      in Wait().
			//      The semaphore value actually reflects the number of Messages in
			//      the queue.
			this->sem.Wait();
			Message* ret = this->first;
			this->first = this->first->next;

			if(this->first == 0){
#if defined(WIN32)
				if(ResetEvent(this->eventForWaitable) == 0){
					ASSERT(false)
					throw ting::Exc("Queue::Wait(): ResetEvent() failed");
				}
#elif defined(__linux__)
				{
					eventfd_t value;
					if(eventfd_read(this->eventFD, &value) < 0){
						throw ting::Exc("Queue::Wait(): eventfd_read() failed");
					}
					ASSERT(value == 1)
				}
#elif defined(__APPLE__)
				{
					u8 oneByteBuf[1];
					read(this->pipeEnds[0], oneByteBuf, 1);
				}
#else
#error "Unsupported OS"
#endif
				this->ClearCanReadFlag();
			}else{
				ASSERT(this->CanRead())
			}

			return Ptr<Message>(ret);
		}
		return Ptr<Message>();
	}



	/**
	 * @brief Get message from queue, blocks if no messages queued.
	 * This method gets a message from message queue. If there are no messages on the queue
	 * it will wait until any message is posted to the queue.
	 * Note, that this method, due to its implementation, is not intended to be called from
	 * multiple threads simultaneously (unlike Queue::PeekMsg()).
	 * If it is called from multiple threads the behavior will be undefined.
	 * It is also forbidden to call GetMsg() from one thread and PeekMsg() from another
	 * thread on the same Queue instance, because it will also lead to undefined behavior.
	 * @return auto-pointer to Message instance.
	 */
	Ptr<Message> GetMsg(){
		M_QUEUE_TRACE(<< "Queue[" << this << "]::GetMsg(): enter" << std::endl)
		{
			Mutex::Guard mutexGuard(this->mut);
			if(this->first){
				ASSERT(this->CanRead())
				//NOTE: Decrement semaphore value, because we take one message from queue.
				//      The semaphore value should be > 0 here, so there will be no hang
				//      in Wait().
				//      The semaphore value actually reflects the number of Messages in
				//      the queue.
				this->sem.Wait();
				Message* ret = this->first;
				this->first = this->first->next;

				if(this->first == 0){
#if defined(WIN32)
					if(ResetEvent(this->eventForWaitable) == 0){
						ASSERT(false)
						throw ting::Exc("Queue::Wait(): ResetEvent() failed");
					}
#elif defined(__linux__)
					{
						eventfd_t value;
						if(eventfd_read(this->eventFD, &value) < 0){
							throw ting::Exc("Queue::Wait(): eventfd_read() failed");
						}
						ASSERT(value == 1)
					}
#elif defined(__APPLE__)
					{
						u8 oneByteBuf[1];
						read(this->pipeEnds[0], oneByteBuf, 1);
					}
#else
#error "Unsupported OS"
#endif
					this->ClearCanReadFlag();
				}else{
					ASSERT(this->CanRead())
				}

				M_QUEUE_TRACE(<< "Queue[" << this << "]::GetMsg(): exit without waiting on semaphore" << std::endl)
				return Ptr<Message>(ret);
			}
		}
		M_QUEUE_TRACE(<< "Queue[" << this << "]::GetMsg(): waiting" << std::endl)
		
		this->sem.Wait();
		
		M_QUEUE_TRACE(<< "Queue[" << this << "]::GetMsg(): signalled" << std::endl)
		{
			Mutex::Guard mutexGuard(this->mut);
			ASSERT(this->CanRead())
			ASSERT(this->first)
			Message* ret = this->first;
			this->first = this->first->next;

			if(this->first == 0){
#if defined(WIN32)
				if(ResetEvent(this->eventForWaitable) == 0){
					ASSERT(false)
					throw ting::Exc("Queue::Wait(): ResetEvent() failed");
				}
#elif defined(__linux__)
				{
					eventfd_t value;
					if(eventfd_read(this->eventFD, &value) < 0){
						throw ting::Exc("Queue::Wait(): eventfd_read() failed");
					}
					ASSERT(value == 1)
				}
#elif defined(__APPLE__)
				{
					u8 oneByteBuf[1];
					read(this->pipeEnds[0], oneByteBuf, 1);
				}
#else
#error "Unsupported OS"
#endif
				this->ClearCanReadFlag();
			}else{
				ASSERT(this->CanRead())
			}

			M_QUEUE_TRACE(<< "Queue[" << this << "]::GetMsg(): exit after waiting on semaphore" << std::endl)
			return Ptr<Message>(ret);
		}
	}

private:
#ifdef WIN32
	//override
	HANDLE GetHandle(){
		//return event handle
		return this->eventForWaitable;
	}

	u32 flagsMask;//flags to wait for

	//override
	virtual void SetWaitingEvents(u32 flagsToWaitFor){
		//It is not allowed to wait on queue for write,
		//because it is always possible to push new message to queue.
		//Error condition is not possible for Queue.
		//Thus, only possible flag values are READ and 0 (NOT_READY)
		if(flagsToWaitFor != 0 && flagsToWaitFor != ting::Waitable::READ){
			ASSERT_INFO(false, "flagsToWaitFor = " << flagsToWaitFor)
			throw ting::Exc("Queue::SetWaitingEvents(): flagsToWaitFor should be ting::Waitable::READ or 0, other values are not allowed");
		}

		this->flagsMask = flagsToWaitFor;
	}

	//returns true if signalled
	virtual bool CheckSignalled(){
		//error condition is not possible for queue
		ASSERT((this->readinessFlags & ting::Waitable::ERROR_CONDITION) == 0)

/*
#ifdef DEBUG
		{
			Mutex::Guard mutexGuard(this->mut);
			if(this->first){
				ASSERT_ALWAYS(this->CanRead())

				//event should be in signalled state
				ASSERT_ALWAYS(WaitForSingleObject(this->eventForWaitable, 0) == WAIT_OBJECT_0)
			}

			if(this->CanRead()){
				ASSERT_ALWAYS(this->first)

				//event should be in signalled state
				ASSERT_ALWAYS(WaitForSingleObject(this->eventForWaitable, 0) == WAIT_OBJECT_0)
			}

			//if event is in signalled state
			if(WaitForSingleObject(this->eventForWaitable, 0) == WAIT_OBJECT_0){
				ASSERT_ALWAYS(this->CanRead())
				ASSERT_ALWAYS(this->first)
			}
		}
#endif
*/

		return (this->readinessFlags & this->flagsMask) != 0;
	}
#elif defined(__linux__)
	//override
	int GetHandle(){
		return this->eventFD;
	}
#elif defined(__APPLE__) //Mac OS
	//override
	int GetHandle(){
		//return read end of pipe
		return this->pipeEnds[0];
	}
#else
#error "Unsupported OS"
#endif
};//~class Queue



/**
 * @brief a base class for threads.
 * This class should be used as a base class for thread objects, one should override the
 * Thread::Run() method.
 */
class Thread{
//Tread Run function
#ifdef WIN32
	static unsigned int __stdcall RunThread(void *data)
#elif defined(__SYMBIAN32__)
	static TInt RunThread(TAny *data)
#elif defined(__linux__) || defined(__APPLE__)
	static void* RunThread(void *data)
#else
#error "Unsupported OS"
#endif
	{
		ting::Thread *thr = reinterpret_cast<ting::Thread*>(data);
		try{
			thr->Run();
		}catch(ting::Exc& e){
			ASSERT_INFO(false, "uncaught ting::Exc exception in Thread::Run(): " << e.What())
		}catch(std::exception& e){
			ASSERT_INFO(false, "uncaught std::exception exception in Thread::Run(): " << e.what())
		}catch(...){
			ASSERT_INFO(false, "uncaught unknown exception in Thread::Run()")
		}

		{
			//protect by mutex to avoid changing the
			//this->state variable before Join() or Start() has finished.
			ting::Mutex::Guard mutexGuard(Thread::Mutex2());
			
			thr->state = STOPPED;
		}

#ifdef WIN32
		//Do nothing, _endthreadex() will be called   automatically
		//upon returning from the thread routine.
#elif defined(__linux__) || defined(__APPLE__)
		pthread_exit(0);
#else
#error "Unsupported OS"
#endif
		return 0;
	}

	ting::Mutex mutex1;

	static inline ting::Mutex& Mutex2(){
		static ting::Mutex m;
		return m;
	}

	enum E_State{
		NEW,
		RUNNING,
		STOPPED,
		JOINED
	} state;

	//system dependent handle
#if defined(WIN32)
	HANDLE th;
#elif defined(__SYMBIAN32__)
	RThread th;
#elif defined(__linux__) || defined(__APPLE__)
	pthread_t th;
#else
#error "Unsupported OS"
#endif

	//forbid copying
	Thread(const Thread& ){
		ASSERT(false)
	}
	
	Thread& operator=(const Thread& ){
		return *this;
	}

public:
	inline Thread() :
			state(Thread::NEW)
	{
#if defined(WIN32)
		this->th = NULL;
#elif defined(__SYMBIAN32__)
		//do nothing
#elif defined(__linux__) || defined(__APPLE__)
		//do nothing
#else
#error "Unsuported OS"
#endif
	}

	virtual ~Thread(){
		ASSERT_INFO(
				this->state == JOINED || this->state == NEW,
				"~Thread() destructor is called while the thread was not joined before. "
				<< "Make sure the thread is joined by calling Thread::Join() "
				<< "before destroying the thread object."
			)

		//NOTE: it is incorrect to put this->Join() to this destructor, because
		//thread shall already be stopped at the moment when this destructor
		//is called. If it is not, then the thread will be still running
		//when part of the thread object is already destroyed, since thread object is
		//usually a derived object from Thread class and the destructor of this derived
		//object will be called before ~Thread() destructor.
	}



	/**
	 * @brief This should be overriden, this is what to be run in new thread.
	 * Pure virtual method, it is called in new thread when thread runs.
	 */
	virtual void Run() = 0;



	/**
	 * @brief Start thread execution.
	 * Starts execution of the thread. Thread's Thread::Run() method will
	 * be run as separate thread of execution.
	 * @param stackSize - size of the stack in bytes which should be allocated for this thread.
	 *                    If stackSize is 0 then system default stack size is used.
	 */
	//0 stacksize stands for default stack size (platform dependent)
	void Start(unsigned stackSize = 0){
		//Protect by mutex to avoid several Start() methods to be called
		//by concurrent threads simultaneously and to protect call to Join() before Start()
		//has returned.
		ting::Mutex::Guard mutexGuard1(this->mutex1);
		//Protect by mutex to avoid incorrect state changing in case when thread
		//exits before the Start() method retruned.
		ting::Mutex::Guard mutexGuard2(Thread::Mutex2());

		if(this->state != NEW)
			throw ting::Exc("Thread::Start(): Thread is already running or stopped");

#ifdef WIN32
		this->th = reinterpret_cast<HANDLE>(
				_beginthreadex(
						NULL,
						unsigned(stackSize),
						&RunThread,
						reinterpret_cast<void*>(this),
						0,
						NULL
					)
			);
		if(this->th == NULL)
			throw ting::Exc("Thread::Start(): starting thread failed");
#elif defined(__SYMBIAN32__)
		if(this->th.Create(_L("ting thread"), &RunThread,
					stackSize == 0 ? KDefaultStackSize : stackSize,
					NULL, reinterpret_cast<TAny*>(this)) != KErrNone
				)
		{
			throw ting::Exc("Thread::Start(): starting thread failed");
		}
		this->th.Resume();//start the thread execution
#elif defined(__linux__) || defined(__APPLE__)
		{
			pthread_attr_t attr;

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
			pthread_attr_setstacksize(&attr, size_t(stackSize));

			int res = pthread_create(&this->th, &attr, &RunThread, this);
			if(res != 0){
				pthread_attr_destroy(&attr);
				TRACE_AND_LOG(<< "Thread::Start(): pthread_create() failed, error code = " << res
						<< " meaning: " << strerror(res) << std::endl)
				std::stringstream ss;
				ss << "Thread::Start(): starting thread failed,"
						<< " error code = " << res << ": " << strerror(res);
				throw ting::Exc(ss.str().c_str());
			}
			pthread_attr_destroy(&attr);
		}
#else
#error "Unsupported OS"
#endif
		this->state = RUNNING;
	}



	/**
	 * @brief Wait for thread to finish its execution.
	 * This functon waits for the thread finishes its execution,
	 * i.e. until the thread returns from its Thread::Run() method.
	 * Note: it is safe to call Join() on not started threads,
	 *       in that case it will return immediately.
	 */
	void Join(){
//		TRACE(<< "Thread::Join(): enter" << std::endl)

		//protect by mutex to avoid several Join() methods to be called by concurrent threads simultaneously
		ting::Mutex::Guard mutexGuard(this->mutex1);

		if(this->state == NEW){
			//thread was not started, do nothing
			return;
		}

		if(this->state == JOINED){
			throw ting::Exc("Thread::Join(): thread is already joined");
		}

		ASSERT(this->state == RUNNING || this->state == STOPPED)

#ifdef WIN32
		WaitForSingleObject(this->th, INFINITE);
		CloseHandle(this->th);
		this->th = NULL;
#elif defined(__SYMBIAN32__)
		TRequestStatus reqStat;
		this->th.Logon(reqStat);
		User::WaitForRequest(reqStat);
		this->th.Close();
#elif defined(__linux__) || defined(__APPLE__)
		pthread_join(this->th, 0);
#else
#error "Unsupported OS"
#endif

		//NOTE: at this point the thread's Run() method should already exit and state
		//should be set to STOPPED
		ASSERT(this->state == STOPPED)

		this->state = JOINED;

//		TRACE(<< "Thread::Join(): exit" << std::endl)
	}



	/**
	 * @brief Suspend the thread for a given number of milliseconds.
	 * Suspends the thread which called this function for a given number of milliseconds.
	 * This function guarantees that the calling thread will be suspended for
	 * AT LEAST 'msec' milliseconds.
	 * @param msec - number of milliseconds the thread should be suspended.
	 */
	static void Sleep(unsigned msec = 0){
#ifdef WIN32
		SleepEx(DWORD(msec), FALSE);// Sleep() crashes on mingw (I do not know why), this is why I use SleepEx() here.
#elif defined(__SYMBIAN32__)
		User::After(msec * 1000);
#elif defined(sun) || defined(__sun) || defined(__APPLE__) || defined(__linux__)
		if(msec == 0){
	#if defined(sun) || defined(__sun) || defined(__APPLE__)
			sched_yield();
	#elif defined(__linux__)
			pthread_yield();
	#else
	#error "Should not get here"
	#endif
		}else{
			usleep(msec * 1000);
		}
#else
#error "Unsupported OS"
#endif
	}



	/**
	 * @brief get current thread ID.
	 * Returns unique identifier of the currently executing thread. This ID can further be used
	 * to make assertions to make sure that some code is executed in a specific thread. E.g.
	 * assert that methods of some object are executed in the same thread where this object was
	 * created.
	 * @return unique thread identifier.
	 */
	static inline ting::ulong GetCurrentThreadID(){
#ifdef WIN32
		return ting::ulong(GetCurrentThreadId());
#elif defined(__APPLE__) || defined(__linux__)
		pthread_t t = pthread_self();
		STATIC_ASSERT(sizeof(pthread_t) <= sizeof(ting::ulong))
		return ting::ulong(t);
#else
#error "Unsupported OS"
#endif
	}
};



/**
 * @brief a thread with message queue.
 * This is just a facility class which already contains message queue and boolean 'quit' flag.
 */
class MsgThread : public Thread{
	friend class QuitMessage;
	
protected:
	/**
	 * @brief Flag indicating that the thread should exit.
	 * This is a flag used to stop thread execution. The implementor of
	 * Thread::Run() method usually would want to use this flag as indicator
	 * of thread exit request. If this flag is set to true then the thread is requested to exit.
	 * The typical usage of the flag is as follows:
	 * @code
	 * class MyThread : public ting::MsgThread{
	 *     ...
	 *     void MyThread::Run(){
	 *         while(!this->quitFlag){
	 *             //get and handle thread messages, etc.
	 *             ...
	 *         }
	 *     }
	 *     ...
	 * };
	 * @endcode
	 */
	ting::Inited<volatile bool, false> quitFlag;//looks like it is not necessary to protect this flag by mutex, volatile will be enough

	Queue queue;

public:
	inline MsgThread(){}

	/**
	 * @brief Send 'Quit' message to thread's queue.
	 */
	inline void PushQuitMessage();//see implementation below



	/**
	 * @brief Send 'no operation' message to thread's queue.
	 */
	inline void PushNopMessage();//see implementation below



	/**
	 * @brief Send a message to thread's queue.
	 * @param msg - a message to send.
	 */
	inline void PushMessage(Ptr<ting::Message> msg){
		this->queue.PushMessage(msg);
	}
};



/**
 * @brief Tells the thread that it should quit its execution.
 * The handler of this message sets the quit flag (Thread::quitFlag)
 * of the thread which this message is sent to.
 */
class QuitMessage : public Message{
	MsgThread *thr;
  public:
	QuitMessage(MsgThread* thread) :
			thr(thread)
	{
		if(!this->thr)
			throw ting::Exc("QuitMessage::QuitMessage(): thread pointer passed is 0");
	}

	//override
	void Handle(){
		this->thr->quitFlag = true;
	}
};



/**
 * @brief do nothing message.
 * The handler of this message dos nothing when the message is handled. This message
 * can be used to unblock thread which is waiting infinitely on its message queue.
 */
class NopMessage : public Message{
  public:
	NopMessage(){}

	//override
	void Handle(){
		//Do nothing, nop
	}
};



inline void MsgThread::PushNopMessage(){
	this->PushMessage(Ptr<Message>(new NopMessage()));
}



inline void MsgThread::PushQuitMessage(){
	//TODO: post preallocated quit message?
	this->PushMessage(Ptr<Message>(new QuitMessage(this)));
}



}//~namespace ting

//NOTE: do not put semicolon after namespace, some compilers issue a warning on this thinking that it is a declaration.



//if Microsoft MSVC compiler, restore warnings state
#ifdef _MSC_VER
#pragma warning(pop) //pop warnings state
#endif


