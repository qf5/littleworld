#include "scheduler.h"

#include "log.h"

namespace qf {
namespace co {

Scheduler::Scheduler(uint32_t threadNum) : m_threadNum(threadNum) {
	for (uint32_t i = 1; i < m_threadNum; i++) {
		auto thread = thread::CreateThread(&Main, this, i);
		m_threads.push_back(thread);
	}
}

void Scheduler::Run() {
	for (auto& thread : m_threads) {
		thread->Run();
	}

	Main(this, 0);

	for (auto& thread : m_threads) {
		thread->Join();
	}
}

util::Func Scheduler::GetFunc(uint32_t threadNo) {
	thread::LockGuard<thread::Mutex> lock(mu);
	for (auto iter =  m_list.begin(); iter != m_list.end(); iter++) {
		if (iter->second == m_maxThreadNo || iter->second == threadNo) {
			util::Func func = iter->first;
			m_list.erase(iter);
			return func;
		}
	}
	return util::Func();
}

void Scheduler::Main(Scheduler* self, uint32_t threadNo) {
	while (true) {
		if (auto func = self->GetFunc(threadNo)) {
			auto co = GetManager()->_create(func);
			Resume(co);
		} else {
			break;
		}
	}
}

}

}
