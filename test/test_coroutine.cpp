#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "coroutine.h"
#include "log.h"
#include <assert.h>

using namespace qf;

static auto logger = GetLogger();

void func(int i) {
	logger->Info("func yield begin.", i);
	co::Yield();
	logger->Info("func yield end.", i);
}

#if 1
int main(int argc, char* argv[])
{
	auto cofunc = co::Create(func, 100);
	auto colambda = co::Create([cofunc]() {
		logger->Info("resume func begin 1.");
		co::Resume(cofunc);
		logger->Info("resume func end 1.");

		logger->Info("resume func begin 2.");
		co::Resume(cofunc);
		logger->Info("resume func end 2.");
	});
	logger->Info("colambda begin.");
	co::Resume(colambda);
	logger->Info("colambda end.");
	return 0;
}
#endif

#if 0
int main(int argc, char* argv[]) {
	auto cofunc = co::Create(func);
	logger->Info("cofunc begin 1.");
	co::Resume(cofunc);
	logger->Info("cofunc end 1.");

	logger->Info("cofunc begin 2.");
	co::Resume(cofunc);
	logger->Info("cofunc end 2.");
}
#endif
