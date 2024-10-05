#pragma once
#include<glog/logging.h>
#include<coroutine>
#include<chrono>
#include<assert.h>
#include<mutex>
#include<functional>

typedef std::function<void()> FunCancel;
typedef std::function<void()> FunRunCurrentCo;
extern FunRunCurrentCo g_funRunCurrentCo;
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
	struct promise_type
	{
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
		struct final_awaiter
		{
			constexpr bool await_ready() noexcept
			{
				//LOG(INFO) << "in final_awaiter.await_ready";
				return false;
			}
			void await_resume() noexcept
			{
				LOG(ERROR) << "in final_awaiter.await_resume";
				assert(false);
			}
			std::coroutine_handle<>
				await_suspend(std::coroutine_handle<promise_type> h) noexcept
			{
				//LOG(INFO) << "in final_awaiter.await_suspend";
				if (auto previous = h.promise().previous; previous)
				{
					//LOG(INFO) << "in final_awaiter.await_suspend.previous";
					return previous;
				}
				else
				{
					//LOG(INFO) << "in final_awaiter.await_suspend.noop_coroutine";
					return std::noop_coroutine();
				}
			}
			std::coroutine_handle<> coro;
		};
		/// <summary>
		/// 协程结束前执行
		/// 如果改成suspend_never就问题很多，不知道为什么
		/// </summary>
		/// <returns></returns>
		final_awaiter final_suspend() noexcept
		{
			return {};
		}
		/// <summary>
		/// 出现未经处理的异常时执行
		/// 这里只知道协程执行异常，却无法知道是什么异常
		/// 只有在每一段co_yield前后都手工加try才能知道具体错
		/// </summary>
		void unhandled_exception()
		{
			//return std::terminate(); 
			LOG(WARNING) << "unhandled_exception";
		}
		// co_return 时执行，return_void跟return_value二选一，协程的返回值
		//void return_void() {}
		void return_value(T v) { value = v; }

		//int return_value(int result) { this.result = reslut; }

		//co_yield时执行
		auto yield_value(T value)
		{
			this->value = value;
			return std::suspend_always{};
		}
		/// <summary>
		/// 自己作为可等待对象
		/// </summary>
		std::coroutine_handle<> previous = nullptr;

	};

	//const std::chrono::duration<int, std::milli> length = 1000ms;
	using handle = std::coroutine_handle<promise_type>;
	/// <summary>
	/// 继续执行协程函数代码执直到遇到yield挂起
	/// </summary>
	bool Run()
	{
		//std::lock_guard lock(m_mutex);
		if (FinishedNoLock())
		{
			LOG(INFO) << m_desc << "协程已结束，不再执行\n";
			return true;
		}
		if (m_hCoroutine == nullptr)
		{
			assert(false);
			return true;
		}
		g_funRunCurrentCo = [this]() {this->Run(); };
		m_hCoroutine.resume();
		return TryClear();
	}
	/// <summary>
	/// Send专用
	/// </summary>
	/// <param name="send"></param>
	//void Run2(const bool& send)
	//{
	//	std::lock_guard lock(m_mutex);
	//	if (send)
	//		return;
	//	if (!m_hCoroutine)
	//		return;

	//	m_hCoroutine.resume();
	//	TryClear();
	//}
	bool TryClear()
	{
		if (!m_hCoroutine.done())
			return false;

		LOG(INFO) << m_desc << "协程已退出" << m_hCoroutine.address();
		m_hCoroutine.destroy();
		m_hCoroutine = nullptr;

		return true;
	}
	bool Finished()
	{
		//std::lock_guard lock(m_mutex);
		return FinishedNoLock();
	}
	bool FinishedNoLock()
	{
		if (m_hCoroutine == nullptr || m_hCoroutine.address() == nullptr)
			return true;

		return m_hCoroutine.done();
	}
	/// <summary>
	/// 同时也是Awaiter
	/// </summary>
	/// <returns></returns>
	constexpr bool await_ready() const noexcept { return false; }
	/// <summary>
	/// 同时也是Awaiter
	/// </summary>
	/// <returns>co_await本对象的返回值,返回的就是协程co_return的值</returns>
	T await_resume() const noexcept
	{
		return this->m_hCoroutine.promise().value;
	}
	/// <summary>
	/// 同时也是Awaiter
	/// </summary>
	/// <param name="h"></param>
	constexpr auto await_suspend(std::coroutine_handle<void> h) noexcept
	{
		//auto t = std::jthread([h, l = length] {
		//	std::this_thread::sleep_for(l);
		//h.resume();
		//	});
		m_hCoroutine.promise().previous = h;
		return m_hCoroutine;
	}
	std::mutex m_mutex;
private:

	/// <summary>
	/// 自己作为协程
	/// </summary>
	handle m_hCoroutine = nullptr;
	CoTask(handle handle) :m_hCoroutine(handle) {}


public:
	/// <summary>
	/// 方便调试
	/// </summary>
	std::string m_desc;
	//int result;
	CoTask() {}

	/// <summary>
	/// 带有移动语言的复制构造函数
	/// </summary>
	/// <param name="other"></param>
	CoTask(CoTask&& other)noexcept :m_hCoroutine(other.m_hCoroutine) { other.m_hCoroutine = nullptr; }

	/// <summary>
	/// 移动语义，这是必须的，协程反回的临时对象是右值引用，传统复制构造函数传入的const&无法修改传进来的对象
	/// </summary>
	/// <param name="other"></param>
	void operator=(CoTask&& other)noexcept
	{
		assert(FinishedNoLock());
		m_hCoroutine = other.m_hCoroutine;
		other.m_hCoroutine = nullptr;
	}
	~CoTask() { if (m_hCoroutine) m_hCoroutine.destroy(); }
	//bool MoveNext() const { return m_hCoroutine && (m_hCoroutine.resume(), !m_hCoroutine.done()); }
	T GetValue() const { return m_hCoroutine.promise().value; }
};


class KeepCancel
{
public:
	KeepCancel(FunCancel& old, bool autoRevert = true) :funCancelOld(old), refFunCancel(old), m_autoRevert(autoRevert)
	{
	}
	~KeepCancel()
	{
		Revert();
	}
	//KeepCancel(KeepCancel& other)=delete;
	KeepCancel(KeepCancel&& other)noexcept :refFunCancel(other.refFunCancel), funCancelOld(other.funCancelOld), m_autoRevert(other.m_autoRevert)
	{
		other.m_autoRevert = false;
		other.funCancelOld = nullptr;
	}
	void operator =(KeepCancel&& other) = delete;
	void Revert()
	{
		if (!m_autoRevert)
		{
			return;
		}
		//LOG(INFO) << "Revert" ;
		refFunCancel = funCancelOld;
		funCancelOld = nullptr;
		m_autoRevert = false;
	}
	FunCancel funCancelOld;
	FunCancel& refFunCancel;
	void operator=(const KeepCancel& other) = delete;
	//{
	//	funCancelOld = refFunCancel = other.refFunCancel;
	//	m_autoRevert = other.m_autoRevert;
	//}
	bool m_autoRevert;
};

template<class T_Result>
struct CoAwaiter
{
	/// <summary>
	/// 
	/// </summary>
	/// <param name="initSn">用引用可以防止传bool进去</param>
	CoAwaiter(const long& initSn, FunCancel& cancel) : m_Kc(cancel, true)
	{
		m_sn = initSn;
	}

	static long GenSn()
	{
		static std::atomic<long> sn(0);
		return sn.fetch_add(1);//关于fetch_add解释，原子替换当前的值为与参数几何相加后的值，表现为原子的post - increment操作：相加后，返回原始值。
	}
	long Sn()const { return m_sn; }
	// await_ready告诉co_await准备好没有，如果返回false，
	//    如果await_suspend返回coroutine_handle的一个实例h，
	//    那么恢复这个handle，即运行await_suspend(h).resume()
	//    即暂停本协程同时恢复另一个协程。
	//    如果await_suspend返回bool，如果为false则恢复自己。
	//    如果await_suspend返回void, 那么直接执行，执行完暂停本协程。
	// 如果await_ready返回true或者协程被恢复了，那么执行await_resume，
	// 执行完暂停本协程。
	// 总结下：
	//    await_ready:准备好了没有。
	//    await_suspend:停不停止。 
	//    await_resume:好了做什么。




	/// <summary>
	/// 
	/// </summary>
	/// <returns>co_await本对象的返回值</returns>
	T_Result await_resume() const noexcept
	{
		return m_Result;
	}
	constexpr bool await_ready() const noexcept { return m_bAwaitReady; }
	constexpr void await_suspend(std::coroutine_handle<> h) noexcept
	{
		//auto t = std::jthread([h, l = length] {
		//	std::this_thread::sleep_for(l);
		//h.resume();
		//	});
		m_hAwaiter = h;
	}

	CoAwaiter(const CoAwaiter& other) = delete;// :m_Kc(other.m_Kc.refFunCancel, false)
	//{
	//	CopyFrom(other);
	//}
	//void CopyFrom(const CoAwaiter& other)
	//{
	//	m_hAwaiter = other.m_hAwaiter;
	//	m_sn = other.m_sn;
	//	//other.m_hAwaiter = nullptr;
	//	//other.m_sn = 0;
	//	m_Result = other.m_Result;
	//	m_Kc = other.m_Kc;
	//}
	CoAwaiter(CoAwaiter&& other) noexcept :m_Kc(std::move(other.m_Kc)), m_sn(other.m_sn), m_hAwaiter(other.m_hAwaiter), m_Result(other.m_Result)
	{
		other.m_hAwaiter = nullptr;
		other.m_sn = 0;
	}
	void operator =(CoAwaiter&& other)noexcept = delete;
	//{
	//	MoveFrom(other);
	//}
	void operator =(const CoAwaiter& other)noexcept = delete;
	//{
	//	CopyFrom(other);
	//}

	void Run(const T_Result& result)
	{
		m_Result = result;
		m_Kc.Revert();
		m_hAwaiter.resume();
	}
	void SetResult(const T_Result& result)
	{
		m_Result = result;
	}
	/// <summary>
	/// 在别的地方已经给m_Result赋值
	/// </summary>
	void Run()
	{
		m_Kc.Revert();
		m_hAwaiter.resume();
	}
	void Run2(const T_Result& result, std::mutex& mutex)
	{
		std::lock_guard lock(mutex);
		Run(result);
	}
	bool m_bAwaitReady = false;
private:
	long m_sn;
	T_Result m_Result;//可等待对象的返回值
	KeepCancel m_Kc;
	std::coroutine_handle<> m_hAwaiter;
};

typedef CoAwaiter<bool> CoAwaiterBool;