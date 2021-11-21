#include <string>
#include "log.h"

static auto logger = qf::GetLogger("system");

template<class T>
using is_bool = util::is_bool<T>;

int main(int argc, char* argv[]) {
	int a = 1;
	logger.Error("this is an error", a);
	logger.Info("hello world", a);
	std::string s = "";
	logger.Info("test string", s.empty());
	auto b1 = is_bool<bool>::value;
	auto b2 = is_bool<const bool>::value;
	logger.Info("type bool", b1, b2);
	return 0;
}
