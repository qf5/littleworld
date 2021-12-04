#include "thread.h"

namespace qf {
namespace thread {

static thread_local uint32_t gtid = 0;

std::atomic<uint32_t> Thread::nid;

void* Thread::ThreadMain(void* arg) {
	Thread* self = (Thread*)arg;
	gtid = ++nid;
	self->func();
	return nullptr;
}

uint32_t GetThreadId() {
	return gtid;
}

}

}
