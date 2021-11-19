#pragma once

#include <memory>

template<class T> std::shared_ptr<T> GetInstance() {
	static std::shared_ptr<T> ptr = std::make_shared<T>();
	return ptr;
}
