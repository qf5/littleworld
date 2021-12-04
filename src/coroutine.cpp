#include <assert.h>
#include "coroutine.h"

namespace qf {
namespace co {

static inline void _comain() {
	auto manager = util::GetInstance<CoManager>();
	auto co = manager->GetRunning();
	assert(co);
	co->func();
	co->status = CoStatus::DEAD;
	manager->DelCo(co->id);
}

const CoroutinePtr CoManager::_create(util::Func& func) {
	auto co = std::make_shared<Coroutine>(func, ++m_coId, this);
	getcontext(&co->ctx);
	co->ctx.uc_stack.ss_sp = co->stack;
	co->ctx.uc_stack.ss_size = co->initStackSize;
	co->ctx.uc_link = &co->octx;
	makecontext(&co->ctx, (void(*)())_comain, 0);
	m_cos.insert(std::make_pair(co->id, co));
	return co;
}

void CoManager::Resume(CoroutinePtr& co) {
	assert(co->status == CoStatus::SUSPENDED);
	co->status = CoStatus::RUNNING;
	auto oco = SetRunning(co);
	swapcontext(&co->octx, &co->ctx);
	SetRunning(oco);
}

void CoManager::Yield() {
	assert(m_running);
	assert(m_running->status == CoStatus::RUNNING);
	m_running->status = CoStatus::SUSPENDED;
	swapcontext(&m_running->ctx, &m_running->octx);
}

void Resume(const CoroutinePtr& co) {
	auto manager = util::GetInstance<CoManager>();
	manager->Resume(const_cast<CoroutinePtr&>(co));
}

void Yield() {
	auto manager = util::GetInstance<CoManager>();
	manager->Yield();
}

const CoroutinePtr Running() {
	auto manager = util::GetInstance<CoManager>();
	return manager->GetRunning();
}

static std::map<CoStatus, const char*> StatusDesc = {
	{ CoStatus::DEAD, "Dead" },
	{ CoStatus::RUNNING, "Running" },
	{ CoStatus::SUSPENDED, "Suspended" },
};

const char* Status(const CoroutinePtr& co) {
	return StatusDesc[co->status];
}

CoManagerPtr GetManager() {
	static thread_local CoManagerPtr manager;
	if (!manager) {
		manager = std::make_shared<CoManager>();
	}
	return manager;
}

}

}
