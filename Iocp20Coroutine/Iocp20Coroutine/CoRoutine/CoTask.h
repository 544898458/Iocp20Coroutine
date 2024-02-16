#pragma once
#include <glog/logging.h>
#include<coroutine>
#include<chrono>
#include<assert.h>
#include<mutex>
//#include <iostream>
using namespace std;

/// <summary>
/// 方便实现协程，没有任何具体逻辑，没有线程，没有网络
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class CoTask
{
public:
	// 协程开始时，在协程的状态对象分配内存后，调用promise_type的构造函数
	struct promise_type {
		T value;
		// 为协程的状态对象分配内存失败时
		//static auto get_return_object_on_allocation_failure() { return CoTask{ nullptr }; }

		// 构造成功后开始执行
		auto get_return_object()
		{
			return CoTask{ handle::from_promise(*this) };
		}
		// 在以上函数后执行
		auto initial_suspend() { return std::suspend_always{}; }
		// 协程结束前执行
		auto final_suspend() noexcept
		{
			return std::suspend_always{}; 
		}
		// 出现未经处理的异常时执行
		void unhandled_exception() 
		{
			//return std::terminate(); 
			LOG(INFO) << "unhandled_exception" ;
		}
		// co_return 时执行，return_void跟return_value二选一
		//void return_void() {}
		void return_value(T v) { value = v; }

		//int return_value(int result) { this.result = reslut; }

		//co_yield时执行
		auto yield_value(T value)
		{
			this->value = value;
			return std::suspend_always{};
		}
	};
	constexpr void await_resume() const noexcept {}
	constexpr bool await_ready() const noexcept { return false; }
	constexpr void await_suspend(std::coroutine_handle<> h) const noexcept
	{
		auto t = std::jthread([h, l = length] {
			std::this_thread::sleep_for(l);
			h.resume();
			});
	}
	const std::chrono::duration<int, std::milli> length = 1000ms;
	using handle = std::coroutine_handle<promise_type>;
	/// <summary>
	/// 继续执行协程函数代码执直到遇到yield挂起
	/// </summary>
	void Run() 
	{ 
		std::lock_guard lock(mutex);
		//if (hCoroutine.done())
		//{
		//	printf("%s协程已结束，不再执行\n", desc.c_str());
		//	return;
		//}
		hCoroutine.resume();
		TryClear();
	}
	void Run2(const bool &send)
	{
		std::lock_guard lock(mutex);
		if (send)
			return;
		hCoroutine.resume();
		//if (hCoroutine.done())
		//{
		//	printf("%s协程已结束，不再执行\n", desc.c_str());
		//	return;
		//}
		TryClear();
	}
	void TryClear()
	{
		if (hCoroutine.done())
		{
			LOG(INFO) << "协程已退出" << hCoroutine.address();
			hCoroutine = nullptr;
		}
	}
	bool Finished()
	{ 
		std::lock_guard lock(mutex);
		if (hCoroutine.address() == nullptr)
			return true;

		return hCoroutine.done(); 
	}
private:
	std::mutex mutex;
	/// <summary>
	/// 唯一的成员变量
	/// </summary>
	handle hCoroutine=nullptr;
	CoTask(handle handle) :hCoroutine(handle) {}
	

public:
	std::string desc;
	//int result;
	CoTask(){}
	//CoTask(CoTask&& other)noexcept :hCoroutine(other.hCoroutine) { other.hCoroutine = nullptr; }
	void operator=(CoTask&& other)noexcept 
	{
		assert(hCoroutine == nullptr);
		hCoroutine = other.hCoroutine;
		other.hCoroutine = nullptr; 
	}
	~CoTask() { if (hCoroutine) hCoroutine.destroy(); }
	//bool MoveNext() const { return hCoroutine && (hCoroutine.resume(), !hCoroutine.done()); }
	T GetValue() const { return hCoroutine.promise().value; }
};
