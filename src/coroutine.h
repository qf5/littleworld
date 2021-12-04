#pragma once

#include <list>
#include <map>
#include <memory.h>
#include <ucontext.h>

#include "util.h"

namespace qf {
namespace co {

enum class CoStatus {
	DEAD = 0,
	RUNNING = 1,
	SUSPENDED = 2,
};

class CoManager;

struct Coroutine {
public:
	Coroutine(util::Func& func, int id, CoManager* manager)
		: func(func)
		, id(id)
		, manager(manager) {

	}

	static constexpr int initStackSize = 16 * 1024 * 1024;
	char stack[initStackSize];
	ucontext_t ctx;
	ucontext_t octx;
	util::Func func;
	int id = -1;
	CoManager* manager;
	CoStatus status = CoStatus::SUSPENDED;
};

typedef std::shared_ptr<Coroutine> CoroutinePtr;

class CoManager {
public:
//	template<class F, class... ArgList>
//	const CoroutinePtr Create(F&& f, ArgList&&... argList) {
//		util::Func func(std::make_shared<util::FuncImpl<F, ArgList...>>(
//				std::forward<F>(f), std::forward<ArgList...>(argList)...));
//		return _create(func);
//	}

	const CoroutinePtr _create(util::Func& func);

	void Resume(CoroutinePtr& co);

	void Yield();

	CoroutinePtr GetCo(int id) {
		auto iter = m_cos.find(id);
		if (iter != m_cos.end()) {
			return iter->second;
		}
		return nullptr;
	}

	void DelCo(int id) {
		m_cos.erase(id);
	}

	CoroutinePtr SetRunning(CoroutinePtr& co) {
		auto oco = m_running;
		m_running = co;
		return oco;
	}

	CoroutinePtr GetRunning() {
		return m_running;
	}

private:
	int m_coId = 0;
	std::map<int, CoroutinePtr> m_cos;
	CoroutinePtr m_running;
};

typedef std::shared_ptr<CoManager> CoManagerPtr;

template<class F, class... ArgList>
const CoroutinePtr Create(F&& f, ArgList&&... argList) {
	auto func = util::CreateFunc(std::forward<F>(f), std::forward<ArgList...>(argList)...);
	auto manager = util::GetInstance<CoManager>();
	return manager->_create(func);
}

void Resume(const CoroutinePtr& co);

void Yield();

const CoroutinePtr Running();

const char* Status(const CoroutinePtr& co);

CoManagerPtr GetManager();

}

}
