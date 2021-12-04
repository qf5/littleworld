#pragma once

#include <list>
#include <memory>

#include "coroutine.h"
#include "thread.h"
#include "util.h"

namespace qf {
namespace co {

class Scheduler {
public:
	Scheduler(uint32_t threadNum = 3);

	template<class F, class... ArgList>
	void Schedule(F&& f, ArgList&&... argList) {
		auto func = util::CreateFunc(std::forward<F>(f), std::forward<ArgList...>(argList)...);
		thread::LockGuard<thread::Mutex> lock(mu);
		m_list.emplace_back(std::move(func), m_maxThreadNo);
	}

	template<class F, class... ArgList>
	void TSchedule(const uint32_t key, F&& f, ArgList&&... argList) {
		auto func = util::CreateFunc(std::forward<F>(f), std::forward<ArgList...>(argList)...);
		thread::LockGuard<thread::Mutex> lock(mu);
		m_list.emplace_back(std::move(func), key % m_threadNum);
	}

	void Run();

private:
	util::Func GetFunc(uint32_t threadNo);

	static void Main(Scheduler* self, uint32_t threadNo);

private:
	std::list<std::pair<util::Func, uint32_t>> m_list;
	thread::Mutex mu;
	const uint32_t m_threadNum;
	const uint32_t m_maxThreadNo = (uint32_t)-1;
	std::list<thread::ThreadPtr> m_threads;
};

}

}
