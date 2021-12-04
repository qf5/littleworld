#include "thread.h"
#include "log.h"
#include <vector>
#include <tuple>
#include <unistd.h>

using namespace qf;

static auto logger = GetLogger();
static thread::Mutex mu;

void thread_entry(int i) {
	thread::LockGuard<thread::Mutex> lock{mu};
	logger->Info("thread running. thread id:", thread::GetThreadId(), "param", i);
	//sleep(5);
}

void test_tuple() {
	int a = 1;
	int& b = a;
	auto tp = std::make_tuple(b);
	logger->Info("value 1", std::get<0>(tp));
	b = 2;
	logger->Info("value 2", std::get<0>(tp));
}

int main(int argc, char* argv[]) {
	std::vector<thread::ThreadPtr> vec;
	for (int i = 0; i < 3; i++) {
		auto thread = thread::CreateThread(thread_entry, i);
		thread->Run();
		vec.push_back(thread);
	}
	for (auto& thd : vec) {
		thd->Join();
	}
	logger->Info("type same", std::is_same<int, typename util::function_traits<decltype(thread_entry)>::arg<0>::type>::value);
	test_tuple();
	return 0;
}
