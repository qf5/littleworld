#include "log.h"

static auto logger = qf::GetLogger();

int main(int argc, char* argv[]) {
	int a = 1;
	logger.Error("this is an error", a);
	logger.Info("hello world", a);
	return 0;
}
