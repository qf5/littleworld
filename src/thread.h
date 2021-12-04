#pragma once

#include <memory>
#include <pthread.h>
#include <atomic>
#include "util.h"

namespace qf {
namespace thread {

class Mutex {
public:
	Mutex() {
		pthread_mutex_init(&m, nullptr);
	}

	~Mutex() {
		pthread_mutex_destroy(&m);
	}

	void Lock() {
		pthread_mutex_lock(&m);
	}

	void Unlock() {
		pthread_mutex_unlock(&m);
	}
	
private:
	pthread_mutex_t m;
};

template<class Mu>
class LockGuard {
public:
	LockGuard(Mu& mu)
		: mu(mu) {
		mu.Lock();
	}

	~LockGuard() {
		mu.Unlock();
	}

private:
	Mu& mu;
};

class TSD {
public:
	TSD(void(*destroy)(void*) = nullptr) {
		pthread_key_create(&key, destroy);
	}

	void * GetSpecific() {
		return pthread_getspecific(key);
	}

	void SetSpecific(void * v) {
		pthread_setspecific(key, v);
	}

private:
	pthread_key_t key;
};

typedef std::shared_ptr<TSD> TSDPtr;

class Thread {
public:
	Thread(util::Func& func) : func(func) {

	}

	int Run() {
		return pthread_create(&tid, nullptr, &ThreadMain, this);
	}

	void Join() {
		pthread_join(tid, nullptr);
	}

	pthread_t GetId() {
		return tid;
	}

private:
	static void* ThreadMain(void* arg);
	static std::atomic<uint32_t> nid;

private:
	util::Func func;
	pthread_t tid;
};

typedef std::shared_ptr<Thread> ThreadPtr;

template<class F, class... ArgList>
ThreadPtr CreateThread(F&& f, ArgList&&... argList) {
	auto func = util::CreateFunc(std::forward<F>(f), std::forward<ArgList>(argList)...);
	return std::make_shared<Thread>(func);
}

uint32_t GetThreadId();

}

}
