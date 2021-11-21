#pragma once

#include <memory>

namespace util
{

template<class T> std::shared_ptr<T> GetInstance() {
	static std::shared_ptr<T> ptr = std::make_shared<T>();
	return ptr;
}

template<class T> struct is_bool {
	static constexpr bool value = std::is_same<typename std::decay<T>::type, bool>::value;
};

}
