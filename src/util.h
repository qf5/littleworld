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

template<class T>
struct function_traits;

template<class R, class... ArgList>
struct function_traits<R(ArgList...)> {
	typedef R return_type;
	typedef std::tuple<ArgList...> arg_type;
	enum {arg_num = sizeof... (ArgList) };
	template<unsigned int N>
	struct arg {
		typedef typename std::tuple_element<N, arg_type>::type type;
	};
};

template<class R, class... ArgList>
struct function_traits<R(*)(ArgList...)> {
	typedef R return_type;
	typedef std::tuple<ArgList...> arg_type;
	enum {arg_num = sizeof... (ArgList) };
	template<unsigned int N>
	struct arg {
		typedef typename std::tuple_element<N, arg_type>::type type;
	};
};

template<class R, class C, class... ArgList>
struct function_traits<R(C::*)(ArgList...)> {
	typedef C class_type;
	typedef R return_type;
	typedef std::tuple<ArgList...> arg_type;
	enum {arg_num = sizeof... (ArgList) };
	template<unsigned int N>
	struct arg {
		typedef typename std::tuple_element<N, arg_type>::type type;
	};
};

template<class R, class C, class... ArgList>
struct function_traits<R(C::*)(ArgList...) const> {
	typedef C class_type;
	typedef R return_type;
	typedef std::tuple<ArgList...> arg_type;
	enum {arg_num = sizeof... (ArgList) };
	template<unsigned int N>
	struct arg {
		typedef typename std::tuple_element<N, arg_type>::type type;
	};
};

class FuncBase {
public:
	virtual void ExecuteFunc() {

	}

	virtual ~FuncBase() {

	}
};

typedef std::shared_ptr<FuncBase> FuncBasePtr;

template<class F>
class FuncImpl : public FuncBase {
public:
	FuncImpl(F&& f) : _f(std::move(f)) {

	}

	virtual void ExecuteFunc() override {
		_f();
	}

private:
	F _f;
};

class Func {
public:
	Func() {

	}

	Func(const FuncBasePtr& func) : _func(func) {

	}

	Func(FuncBasePtr&& func) : _func(std::move(func)) {

	}

	typedef FuncBase * BoolType;

	operator BoolType() {
		return _func == nullptr ? 0 : _func.get();
	}

	bool operator !() {
		return _func == nullptr;
	}

	void operator()() {
		_func->ExecuteFunc();
	}

private:
	FuncBasePtr _func;
};

template<class F, class Args, size_t... N>
void UnpackArgCall(F&& f, Args&& args, std::index_sequence<N...>) {
	f(std::get<N>(args)...);
}

template<class F, class... ArgList>
Func CreateFunc(F&& f, ArgList&&... argList) {
	//注意这里make_tuple是传值 而非引用
	auto args = std::make_tuple(std::forward<ArgList>(argList)...);
	auto wrapper = [f = std::forward<F>(f), args = std::move(args)]() {
		constexpr size_t n = sizeof... (ArgList);
		UnpackArgCall(f, args, std::make_index_sequence<n>{});
	};
	return Func { std::make_shared<FuncImpl<typename std::decay<decltype(wrapper)>::type>>(
			std::move(wrapper)) };
}

}
