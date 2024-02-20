#pragma once
#include<glog/logging.h>
#include<coroutine>
#include<chrono>
#include<assert.h>
#include<mutex>
using namespace std;

/// <summary>
/// 方便实现协程，没有任何具体逻辑，没有线程，没有网络
/// 命名参考C#的ETTask或UniTask，C++20协程标准采用的是微软方案，因此命名也使用C#名字
/// </summary>
/// <typeparam name="T">返回值</typeparam>
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
		
		/// <summary>
		/// 在以上函数后执行
		/// suspend_always表示协程创建后立刻挂起，不执行任何代码，等着用户执行resume
		/// </summary>
		/// <returns></returns>
		auto initial_suspend() { return std::suspend_always{}; }
		
		/// <summary>
		/// 协程结束前执行
		/// 如果改成suspend_never就问题很多，不知道为什么
		/// </summary>
		/// <returns></returns>
		auto final_suspend() noexcept
		{
			return std::suspend_always{}; 
		}
		/// <summary>
		/// 出现未经处理的异常时执行
		/// 这里只知道协程执行异常，却无法知道是什么异常
		/// 只有在每一段co_yield前后都手工加try才能知道具体错
		/// </summary>
		void unhandled_exception() 
		{
			//return std::terminate(); 
			LOG(WARNING) << "unhandled_exception" ;
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
		std::lock_guard lock(m_mutex);
		//if (m_hCoroutine.done())
		//{
		//	printf("%s协程已结束，不再执行\n", m_desc.c_str());
		//	return;
		//}
		if (!m_hCoroutine)
			return;
		m_hCoroutine.resume();
		TryClear();
	}
	/// <summary>
	/// Send专用
	/// </summary>
	/// <param name="send"></param>
	void Run2(const bool &send)
	{
		std::lock_guard lock(m_mutex);
		if (send)
			return;
		if (!m_hCoroutine)
			return;

		m_hCoroutine.resume();
		TryClear();
	}
	void TryClear()
	{
		if (m_hCoroutine.done())
		{
			LOG(INFO) << m_desc <<"协程已退出" << m_hCoroutine.address();
			m_hCoroutine.destroy();
			m_hCoroutine = nullptr;
		}
	}
	bool Finished()
	{ 
		std::lock_guard lock(m_mutex);
		if (m_hCoroutine.address() == nullptr)
			return true;

		return m_hCoroutine.done(); 
	}
private:
	std::mutex m_mutex;
	/// <summary>
	/// 唯一的成员变量
	/// </summary>
	handle m_hCoroutine=nullptr;
	CoTask(handle handle) :m_hCoroutine(handle) {}
	

public:
	/// <summary>
	/// 方便调试
	/// </summary>
	std::string m_desc;
	//int result;
	CoTask(){}
	//CoTask(CoTask&& other)noexcept :m_hCoroutine(other.m_hCoroutine) { other.m_hCoroutine = nullptr; }
	
	/// <summary>
	/// 移动语义，这是必须的，协程反悔的临时对象是右值引用，传统复制构造函数传入的const&无法修改传进来的对象
	/// </summary>
	/// <param name="other"></param>
	void operator=(CoTask&& other)noexcept 
	{
		assert(m_hCoroutine == nullptr);
		m_hCoroutine = other.m_hCoroutine;
		other.m_hCoroutine = nullptr; 
	}
	~CoTask() { if (m_hCoroutine) m_hCoroutine.destroy(); }
	//bool MoveNext() const { return m_hCoroutine && (m_hCoroutine.resume(), !m_hCoroutine.done()); }
	T GetValue() const { return m_hCoroutine.promise().value; }
};
