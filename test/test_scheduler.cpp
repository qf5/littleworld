#include "log.h"
#include "scheduler.h"

using namespace qf;

static auto logger = GetLogger();

static int n = 0;

void test_func(int j) {
	logger->Info(j);
	for (int i = 0; i < 100000; i++) {
		n++;
	}
}

int main(int argc, char* argv[]) {
	int a = 12345;
	co::Scheduler sc;
	sc.TSchedule(1, &test_func, a);
	sc.TSchedule(1, &test_func, a);
	sc.Run();
	logger->Info("Result", n);
	return 0;
}
