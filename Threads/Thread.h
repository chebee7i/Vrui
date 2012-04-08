/***********************************************************************
Thread - Wrapper class for pthreads threads, mostly providing more
convenient thread starting methods and "resource allocation as creation"
paradigm.
Copyright (c) 2005 Oliver Kreylos

This file is part of the Portable Threading Library (Threads).

The Portable Threading Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Portable Threading Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Threading Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef THREADS_THREAD_INCLUDED
#define THREADS_THREAD_INCLUDED

#include <pthread.h>
#include <stdexcept>

namespace Threads {

class ThreadAlreadyRunningError:public std::runtime_error // Exception class thrown when attempting to start a thread that is already running
	{
	/* Constructors and destructors: */
	public:
	ThreadAlreadyRunningError(void)
		:std::runtime_error("Threads::Thread: Attempted to start thread that is already running")
		{
		}
	};

class ThreadAlreadyJoinedError:public std::runtime_error // Exception class thrown when attempting to join a thread that has already been joined
	{
	/* Constructors and destructors: */
	public:
	ThreadAlreadyJoinedError(void)
		:std::runtime_error("Threads::Thread: Attempted to join thread that has already been joined")
		{
		}
	};

class Thread
	{
	/* Embedded classes: */
	public:
	enum CancelState // Cancellation states for threads
		{
		CANCEL_ENABLE=PTHREAD_CANCEL_ENABLE,
		CANCEL_DISABLE=PTHREAD_CANCEL_DISABLE
		};
	
	enum CancelType // Cancellation types for threads
		{
		CANCEL_DEFERRED=PTHREAD_CANCEL_DEFERRED,
		CANCEL_ASYNCHRONOUS=PTHREAD_CANCEL_ASYNCHRONOUS
		};
	
	private:
	class FunctionThreadWrapperArgument // Structure passed to thread wrapper for functions without arguments
		{
		/* Embedded classes: */
		public:
		typedef void* (*ThreadFunction)(void); // Type for thread function
		
		/* Elements: */
		private:
		ThreadFunction threadFunction; // Function to call from thread
		
		/* Constructors and destructors: */
		public:
		FunctionThreadWrapperArgument(ThreadFunction sThreadFunction)
			:threadFunction(sThreadFunction)
			{
			}
		
		/* Methods: */
		static void* wrapper(void* wrapperArgument)
			{
			/* Copy relevant data from wrapper argument and delete it: */
			FunctionThreadWrapperArgument* ftwa=static_cast<FunctionThreadWrapperArgument*>(wrapperArgument);
			ThreadFunction tf=ftwa->threadFunction;
			delete ftwa;
			
			return tf();
			}
		};
	
	template <class ThreadArgumentParam>
	class FunctionArgumentThreadWrapperArgument // Structure passed to thread wrapper for functions with one argument
		{
		/* Embedded classes: */
		public:
		typedef ThreadArgumentParam ThreadArgument;
		typedef void* (*ThreadFunction)(ThreadArgument); // Type for thread function
		
		/* Elements: */
		private:
		ThreadFunction threadFunction; // Function to call from thread
		ThreadArgument threadArgument; // Argument to pass to function to call from thread
		
		/* Constructors and destructors: */
		public:
		FunctionArgumentThreadWrapperArgument(ThreadFunction sThreadFunction,ThreadArgument sThreadArgument)
			:threadFunction(sThreadFunction),threadArgument(sThreadArgument)
			{
			}
		
		/* Methods: */
		static void* wrapper(void* wrapperArgument)
			{
			/* Copy relevant data from wrapper argument and delete it: */
			FunctionArgumentThreadWrapperArgument* fatwa=static_cast<FunctionArgumentThreadWrapperArgument*>(wrapperArgument);
			ThreadFunction tf=fatwa->threadFunction;
			ThreadArgument ta=fatwa->threadArgument;
			delete fatwa;
			
			return tf(ta);
			}
		};
	
	template <class ThreadObjectParam>
	class MethodThreadWrapperArgument // Structure passed to thread wrapper for methods without arguments
		{
		/* Embedded classes: */
		public:
		typedef ThreadObjectParam ThreadObject;
		typedef void* (ThreadObject::*ThreadMethod)(void); // Type for thread method
		
		/* Elements: */
		private:
		ThreadObject* threadObject; // Object to call from thread
		ThreadMethod threadMethod; // Method to call from thread
		
		/* Constructors and destructors: */
		public:
		MethodThreadWrapperArgument(ThreadObject* sThreadObject,ThreadMethod sThreadMethod)
			:threadObject(sThreadObject),threadMethod(sThreadMethod)
			{
			}
		
		/* Methods: */
		static void* wrapper(void* wrapperArgument)
			{
			/* Copy relevant data from wrapper argument and delete it: */
			MethodThreadWrapperArgument* mtwa=static_cast<MethodThreadWrapperArgument*>(wrapperArgument);
			ThreadObject* to=mtwa->threadObject;
			ThreadMethod tm=mtwa->threadMethod;
			delete mtwa;
			
			return (to->*tm)();
			}
		};
	
	template <class ThreadObjectParam,class ThreadArgumentParam>
	struct MethodArgumentThreadWrapperArgument // Structure passed to thread wrapper for methods with one argument
		{
		/* Embedded classes: */
		public:
		typedef ThreadObjectParam ThreadObject;
		typedef ThreadArgumentParam ThreadArgument;
		typedef void* (ThreadObject::*ThreadMethod)(ThreadArgument); // Type for thread method
		
		/* Elements: */
		private:
		ThreadObject* threadObject; // Object to call from thread
		ThreadMethod threadMethod; // Method to call from thread
		ThreadArgument threadArgument; // Argument to pass to method to call from thread
		
		/* Constructors and destructors: */
		public:
		MethodArgumentThreadWrapperArgument(ThreadObject* sThreadObject,ThreadMethod sThreadMethod,ThreadArgument sThreadArgument)
			:threadObject(sThreadObject),threadMethod(sThreadMethod),threadArgument(sThreadArgument)
			{
			}
		
		/* Methods: */
		static void* wrapper(void* wrapperArgument)
			{
			/* Copy relevant data from wrapper argument and delete it: */
			MethodArgumentThreadWrapperArgument* matwa=static_cast<MethodArgumentThreadWrapperArgument*>(wrapperArgument);
			ThreadObject* to=matwa->threadObject;
			ThreadMethod tm=matwa->threadMethod;
			ThreadArgument ta=matwa->threadArgument;
			delete matwa;
			
			return (to->*tm)(ta);
			}
		};
	
	/* Elements: */
	pthread_t threadId; // ID of the thread represented by this object
	bool joined; // Flag if the thread has already been joined by a call to join()
	
	/* Constructors and destructors: */
	public:
	Thread(void) // Creates non-started thread
		:joined(true)
		{
		}
	Thread(void* (*threadFunction)(void),pthread_attr_t* threadAttributes =0) // Creates thread from function with no arguments
		:joined(true)
		{
		/* Start the thread: */
		start(threadFunction,threadAttributes);
		}
	template <class ThreadArgumentParam>
	Thread(void* (*threadFunction)(ThreadArgumentParam),ThreadArgumentParam threadArgument,pthread_attr_t* threadAttributes =0) // Creates thread from function with one argument
		:joined(true)
		{
		/* Start the thread: */
		start(threadFunction,threadArgument,threadAttributes);
		}
	template <class ThreadObjectParam>
	Thread(ThreadObjectParam* threadObject,void* (ThreadObjectParam::*threadMethod)(void),pthread_attr_t* threadAttributes =0) // Creates thread from method without arguments, called on given object
		:joined(true)
		{
		/* Start the thread: */
		start(threadObject,threadMethod,threadAttributes);
		}
	template <class ThreadObjectParam,class ThreadArgumentParam>
	Thread(ThreadObjectParam* threadObject,void* (ThreadObjectParam::*threadMethod)(ThreadArgumentParam),ThreadArgumentParam threadArgument,pthread_attr_t* threadAttributes =0) // Creates thread from method with one argument, called on given object
		:joined(true)
		{
		/* Start the thread: */
		start(threadObject,threadMethod,threadArgument,threadAttributes);
		}
	private:
	Thread(const Thread& source); // Prohibit copy constructor
	Thread& operator=(const Thread& source); // Prohibit assignment operator
	public:
	~Thread(void) // Blocks until the thread terminated, then deletes it
		{
		if(!joined)
			{
			/* Wait for the thread to finish: */
			pthread_join(threadId,0);
			}
		}
	
	/* Methods: */
	
	/* Methods called on thread objects: */
	void start(void* (*threadFunction)(void),pthread_attr_t* threadAttributes =0) // Creates thread from function with no arguments
		{
		/* Throw an exception if the thread is already running: */
		if(!joined)
			throw ThreadAlreadyRunningError();
		
		/* Create an argument for the function thread wrapper: */
		typedef FunctionThreadWrapperArgument WrapperArgument;
		WrapperArgument* ftwa=new WrapperArgument(threadFunction);
		
		/* Create the thread and call the function thread wrapper: */
		joined=false;
		pthread_create(&threadId,threadAttributes,WrapperArgument::wrapper,ftwa);
		}
	template <class ThreadArgumentParam>
	void start(void* (*threadFunction)(ThreadArgumentParam),ThreadArgumentParam threadArgument,pthread_attr_t* threadAttributes =0) // Creates thread from function with one argument
		{
		/* Throw an exception if the thread is already running: */
		if(!joined)
			throw ThreadAlreadyRunningError();
		
		/* Create an argument for the function thread wrapper: */
		typedef FunctionArgumentThreadWrapperArgument<ThreadArgumentParam> WrapperArgument;
		WrapperArgument* fatwa=new WrapperArgument(threadFunction,threadArgument);
		
		/* Create the thread and call the function thread wrapper: */
		joined=false;
		pthread_create(&threadId,threadAttributes,WrapperArgument::wrapper,fatwa);
		}
	template <class ThreadObjectParam>
	void start(ThreadObjectParam* threadObject,void* (ThreadObjectParam::*threadMethod)(void),pthread_attr_t* threadAttributes =0) // Creates thread from method without arguments, called on given object
		{
		/* Throw an exception if the thread is already running: */
		if(!joined)
			throw ThreadAlreadyRunningError();
		
		/* Create an argument for the method thread wrapper: */
		typedef MethodThreadWrapperArgument<ThreadObjectParam> WrapperArgument;
		WrapperArgument* mtwa=new WrapperArgument(threadObject,threadMethod);
		
		/* Create the thread and call the method thread wrapper: */
		joined=false;
		pthread_create(&threadId,threadAttributes,WrapperArgument::wrapper,mtwa);
		}
	template <class ThreadObjectParam,class ThreadArgumentParam>
	void start(ThreadObjectParam* threadObject,void* (ThreadObjectParam::*threadMethod)(ThreadArgumentParam),ThreadArgumentParam threadArgument,pthread_attr_t* threadAttributes =0) // Creates thread from method with one argument, called on given object
		{
		/* Throw an exception if the thread is already running: */
		if(!joined)
			throw ThreadAlreadyRunningError();
		
		/* Create an argument for the method thread wrapper: */
		typedef MethodArgumentThreadWrapperArgument<ThreadObjectParam,ThreadArgumentParam> WrapperArgument;
		WrapperArgument* matwa=new WrapperArgument(threadObject,threadMethod,threadArgument);
		
		/* Create the thread and call the method thread wrapper: */
		joined=false;
		pthread_create(&threadId,threadAttributes,WrapperArgument::wrapper,matwa);
		}
	void detach(void) // Detaches the thread so that its resources are freed immediately upon termination
		{
		/* Throw an exception if the thread is already joined: */
		if(joined)
			throw ThreadAlreadyJoinedError();
		
		/* Detach the thread and mark it as joined: */
		pthread_detach(threadId);
		joined=true;
		}
	void cancel(void) // Requests cancellation of the thread
		{
		pthread_cancel(threadId);
		}
	bool isJoined(void) const // Returns if the thread has already been joined
		{
		return joined;
		}
	void* join(void) // Blocks until the thread terminates, returns its result
		{
		/* Throw an exception if the thread is already joined: */
		if(joined)
			throw ThreadAlreadyJoinedError();
		
		/* Join the thread and return its result: */
		void* result;
		pthread_join(threadId,&result);
		joined=true;
		return result;
		}

	/* Methods called from inside the thread: */
	static CancelState setCancelState(CancelState newCancelState) // Sets the cancellation state of the calling thread; returns old cancellation state
		{
		int oldCancelState;
		pthread_setcancelstate(newCancelState,&oldCancelState);
		return CancelState(oldCancelState);
		}
	static CancelType setCancelType(CancelType newCancelType) // Sets the cancellation type of the calling thread; returns old cancellation type
		{
		int oldCancelType;
		pthread_setcanceltype(newCancelType,&oldCancelType);
		return CancelType(oldCancelType);
		}
	static void testCancel(void) // Tests if the calling thread has a pending cancellation request
		{
		pthread_testcancel();
		}
	static void exit(void* returnValue) // Terminates the calling thread and returns the given value
		{
		pthread_exit(returnValue);
		}
	};

}

#endif
