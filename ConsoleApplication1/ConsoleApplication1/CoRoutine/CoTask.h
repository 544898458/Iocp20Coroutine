#pragma once
#include <glog/logging.h>
#include<coroutine>
#include<chrono>
#include<assert.h>
#include<mutex>
//#include <iostream>
using namespace std;

/// <summary>
/// ����ʵ��Э�̣�û���κξ����߼���û���̣߳�û������
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class CoTask
{
public:
	// Э�̿�ʼʱ����Э�̵�״̬��������ڴ�󣬵���promise_type�Ĺ��캯��
	struct promise_type {
		T value;
		// ΪЭ�̵�״̬��������ڴ�ʧ��ʱ
		//static auto get_return_object_on_allocation_failure() { return CoTask{ nullptr }; }

		// ����ɹ���ʼִ��
		auto get_return_object()
		{
			return CoTask{ handle::from_promise(*this) };
		}
		// �����Ϻ�����ִ��
		auto initial_suspend() { return std::suspend_always{}; }
		// Э�̽���ǰִ��
		auto final_suspend() noexcept
		{
			return std::suspend_always{}; 
		}
		// ����δ��������쳣ʱִ��
		void unhandled_exception() 
		{
			//return std::terminate(); 
			LOG(INFO) << "unhandled_exception" ;
		}
		// co_return ʱִ�У�return_void��return_value��ѡһ
		//void return_void() {}
		void return_value(T v) { value = v; }

		//int return_value(int result) { this.result = reslut; }

		//co_yieldʱִ��
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
	/// ����ִ��Э�̺�������ֱִ������yield����
	/// </summary>
	void Run() 
	{ 
		std::lock_guard lock(mutex);
		//if (hCoroutine.done())
		//{
		//	printf("%sЭ���ѽ���������ִ��\n", desc.c_str());
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
		//	printf("%sЭ���ѽ���������ִ��\n", desc.c_str());
		//	return;
		//}
		TryClear();
	}
	void TryClear()
	{
		if (hCoroutine.done())
		{
			LOG(INFO) << "Э�����˳�" << hCoroutine.address();
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
	/// Ψһ�ĳ�Ա����
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
