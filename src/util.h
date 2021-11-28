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

class FuncBase {
public:
	virtual void ExecuteFunc() {

	}

	virtual ~FuncBase() {

	}
};

typedef std::shared_ptr<FuncBase> FuncBasePtr;

template<class F, class... ArgList>
class FuncImpl : public FuncBase {
public:
	FuncImpl(F&& f, ArgList&&... argList) : _f(std::forward<F>(f)) {
		_args = std::make_tuple<ArgList...>(std::forward<ArgList>(argList)...);
	}

	template<size_t... N>
	void UnpackArgCall(std::index_sequence<N...>) {
		_f(std::get<N>(_args)...);
	}

	virtual void ExecuteFunc() override {
		constexpr size_t n = std::tuple_size<std::tuple<ArgList...>>::value;
		UnpackArgCall(std::make_index_sequence<n>{});
	}

private:
	F _f;
	std::tuple<ArgList...> _args;
};

class Func {
public:
	Func(const FuncBasePtr& func) : _func(func) {

	}

	Func(const FuncBasePtr&& func) : _func(std::move(func)) {

	}

	void operator()() {
		_func->ExecuteFunc();
	}

private:
	FuncBasePtr _func;
};

}
