/*
Э�̵���
CoTask��CoAwaiter������
CoTask�ķ���ֵ��co_return��co_yield�ķ���ֵ������ģ�������MoveNext����
CoAwaiter�ķ���ֵ��co_return�ķ���ֵ��ֱ�ӷ��ظ���һ��Э�̱�����
*/
#pragma once
#include<glog/logging.h>
#include<coroutine>
#include<chrono>
#include<assert.h>
#include<mutex>
#include<functional>

typedef std::function<void()> FunCancel;
/// <summary>
/// ����ʱ�Զ�����ȡ��
/// </summary>
struct FunCancel��ȫ final
{
	FunCancel��ȫ(const std::string& strDebugInfo) :m_strDebugInfo(strDebugInfo)
	{
	}
	void TryCancel()
	{
		if (m_funCancel)
		{
			m_funCancel();
			_ASSERT(!m_funCancel);
			m_funCancel = nullptr;
		}
	}
	~FunCancel��ȫ()
	{
		if (m_funCancel)
		{
			LOG(ERROR) << "����ȡ��:" << m_strDebugInfo;
			_ASSERT(false);
		}
		TryCancel();
	}
	operator FunCancel& ()
	{
		return m_funCancel;
	}
	operator bool()const {
		return m_funCancel.operator bool();
	}
	FunCancel m_funCancel;
	std::string m_strDebugInfo;
};
typedef std::function<void()> FunRunCurrentCo;
extern FunRunCurrentCo g_funRunCurrentCo;
/// <summary>
/// ����ʵ��Э�̣�û���κξ����߼���û���̣߳�û������
/// �����ο�C#��ETTask��UniTask��C++20Э�̱�׼���õ���΢�������������Ҳʹ��C#����
/// </summary>
/// <typeparam name="T">����ֵ</typeparam>
template<typename T>
class CoTask
{
public:
	// Э�̿�ʼʱ����Э�̵�״̬��������ڴ�󣬵���promise_type�Ĺ��캯��
	struct promise_type
	{
		T value;
		// ΪЭ�̵�״̬��������ڴ�ʧ��ʱ
		//static auto get_return_object_on_allocation_failure() { return CoTask{ nullptr }; }

		// ����ɹ���ʼִ��
		auto get_return_object()
		{
			return CoTask{ handle::from_promise(*this) };
		}

		/// <summary>
		/// �����Ϻ�����ִ��
		/// suspend_always��ʾЭ�̴��������̹��𣬲�ִ���κδ��룬�����û�ִ��resume
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
				_ASSERT(false);
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
		/// Э�̽���ǰִ��
		/// ����ĳ�suspend_never������ܶ࣬��֪��Ϊʲô
		/// </summary>
		/// <returns></returns>
		final_awaiter final_suspend() noexcept
		{
			return {};
		}
		/// <summary>
		/// ����δ��������쳣ʱִ��
		/// ����ֻ֪��Э��ִ���쳣��ȴ�޷�֪����ʲô�쳣
		/// ֻ����ÿһ��co_yieldǰ���ֹ���try����֪�������
		/// </summary>
		void unhandled_exception()
		{
			//return std::terminate(); 
			LOG(WARNING) << "unhandled_exception";
			_ASSERT(false);
		}
		// co_return ʱִ�У�return_void��return_value��ѡһ��Э�̵ķ���ֵ
		//void return_void() {}
		void return_value(T v) { value = v; }

		//int return_value(int result) { this.result = reslut; }

		//co_yieldʱִ��
		auto yield_value(T value)
		{
			this->value = value;
			return std::suspend_always{};
		}
		/// <summary>
		/// �Լ���Ϊ�ɵȴ�����
		/// </summary>
		std::coroutine_handle<> previous = nullptr;

	};

	//const std::chrono::duration<int, std::milli> length = 1000ms;
	using handle = std::coroutine_handle<promise_type>;
	/// <summary>
	/// ����ִ��Э�̺�������ֱִ������yield����
	/// </summary>
	bool Run()
	{
		if (m_bNeedLock)
		{
			std::lock_guard lock(m_mutex);
			return RunNoLock();
		}
		else
		{
			return RunNoLock();
		}
	}
	bool RunNoLock()
	{
		if (FinishedNoLock())
		{
			LOG(INFO) << m_desc << "Э���ѽ���������ִ��";
			return true;
		}
		if (m_hCoroutine == nullptr)
		{
			LOG(INFO) << "m_hCoroutine == nullptr";
			_ASSERT(false);
			return true;
		}

		m_hCoroutine.resume();
		return TryClear();
	}

	bool TryClear()
	{
		if (!m_hCoroutine.done())
			return false;

		//LOG(INFO) << m_desc << "Э�����˳�" << m_hCoroutine.address();
		TrySavePromiseValue();
		m_hCoroutine.destroy();
		m_hCoroutine = nullptr;

		return true;
	}
	bool Finished()
	{
		if (m_bNeedLock)
		{
			std::lock_guard lock(m_mutex);
			return FinishedNoLock();
		}
		else {
			return FinishedNoLock();
		}
	}
	bool FinishedNoLock()
	{
		if (m_hCoroutine == nullptr || m_hCoroutine.address() == nullptr)
			return true;

		return m_hCoroutine.done();
	}
	/// <summary>
	/// ͬʱҲ��Awaiter
	/// </summary>
	/// <returns></returns>
	constexpr bool await_ready() const noexcept { return false; }
	/// <summary>
	/// ͬʱҲ��Awaiter
	/// </summary>
	/// <returns>co_await������ķ���ֵ,���صľ���Э��co_return��ֵ</returns>
	T await_resume() const noexcept
	{
		return this->m_hCoroutine.promise().value;
	}
	/// <summary>
	/// ͬʱҲ��Awaiter
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
	/// <summary>
	/// ���߳�(�����߳�)���е�Э��,������������޸���ͬЭ�̲��е�,���������������,����Ӧ���Լ�����CoTask�ж���һ��Э���Ƿ����
	/// </summary>
	void RunNew()
	{
		Run();
		m_listCo.push_back(std::forward<CoTask&&>(*this));
	}
	static void Process()
	{

		const auto oldSize = m_listCo.size();
		std::erase_if(m_listCo, [](CoTask& refCo)->bool {return refCo.Finished(); });
		const auto newSize = m_listCo.size();
		const uint16_t ������ = 50;
		if (oldSize % ������ == 0 || newSize % ������ == 0)
		{
			LOG(INFO) << "oldSize:" << oldSize << ",newSize:" << newSize;
		}
	}
	static void OnAppExit()
	{

	}
	static std::list<CoTask> m_listCo;
	std::mutex m_mutex;
	bool m_bNeedLock = false;
private:

	/// <summary>
	/// �Լ���ΪЭ��
	/// </summary>
	handle m_hCoroutine = nullptr;
	CoTask(handle handle) :m_hCoroutine(handle) {}


public:
	/// <summary>
	/// �������
	/// </summary>
	std::string m_desc;
	//int result;
	CoTask() {}

	/// <summary>
	/// �����ƶ����Եĸ��ƹ��캯��
	/// </summary>
	/// <param name="other"></param>
	CoTask(CoTask&& other)noexcept
	{
		//m_hCoroutine = other.m_hCoroutine;
		//other.m_hCoroutine = nullptr; 
		operator = (std::forward<CoTask&&>(other));//20250304
	}

	/// <summary>
	/// �ƶ����壬���Ǳ���ģ�Э�̷��ص���ʱ��������ֵ���ã���ͳ���ƹ��캯�������const&�޷��޸Ĵ������Ķ���
	/// </summary>
	/// <param name="other"></param>
	void operator=(CoTask&& other)noexcept
	{
		LOG_IF(ERROR, !FinishedNoLock()) << "";
		_ASSERT(FinishedNoLock());
		if (m_hCoroutine)
		{
			TryClear();//20241212
		}
		m_hCoroutine = other.m_hCoroutine;
		other.m_hCoroutine = nullptr;
	}
	~CoTask() { if (m_hCoroutine) m_hCoroutine.destroy(); }
	//bool MoveNext() const { return m_hCoroutine && (m_hCoroutine.resume(), !m_hCoroutine.done()); }
	T GetValue()
	{
		TrySavePromiseValue();
		return m_Value;
	}
	void TrySavePromiseValue()
	{
		if (m_hCoroutine)
			m_Value = m_hCoroutine.promise().value;
	}
	T m_Value;
};
template<typename T>
std::list<CoTask<T>> CoTask<T>::m_listCo;



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
	bool m_autoRevert;
};

template<class T_Result>
struct CoAwaiter
{
	/// <summary>
	/// 
	/// </summary>
	/// <param name="initSn">�����ÿ��Է�ֹ��bool��ȥ</param>
	CoAwaiter(const long& initSn, FunCancel& cancel, const std::string& strDebugInfo = "") : m_Kc(cancel, true), m_strDebugInfo(strDebugInfo)
	{
		m_sn = initSn;
	}

	static long GenSn()
	{
		static std::atomic<long> sn(0);
		return sn.fetch_add(1);//����fetch_add���ͣ�ԭ���滻��ǰ��ֵΪ�����������Ӻ��ֵ������Ϊԭ�ӵ�post - increment��������Ӻ󣬷���ԭʼֵ��
	}
	long Sn()const { return m_sn; }
	// await_ready����co_await׼����û�У��������false��
	//    ���await_suspend����coroutine_handle��һ��ʵ��h��
	//    ��ô�ָ����handle��������await_suspend(h).resume()
	//    ����ͣ��Э��ͬʱ�ָ���һ��Э�̡�
	//    ���await_suspend����bool�����Ϊfalse��ָ��Լ���
	//    ���await_suspend����void, ��ôֱ��ִ�У�ִ������ͣ��Э�̡�
	// ���await_ready����true����Э�̱��ָ��ˣ���ôִ��await_resume��
	// ִ������ͣ��Э�̡�
	// �ܽ��£�
	//    await_ready:׼������û�С�
	//    await_suspend:ͣ��ֹͣ�� 
	//    await_resume:������ʲô��




	/// <summary>
	/// 
	/// </summary>
	/// <returns>co_await������ķ���ֵ</returns>
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
	CoAwaiter(CoAwaiter&& other) noexcept :m_Kc(std::move(other.m_Kc)), m_sn(other.m_sn), m_hAwaiter(other.m_hAwaiter), m_Result(other.m_Result), m_strDebugInfo(other.m_strDebugInfo)
	{
		other.m_hAwaiter = nullptr;
		other.m_sn = 0;
		other.m_strDebugInfo.clear();
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
		try {
			m_hAwaiter.resume();
		}
		catch (const std::exception& e) {
			LOG(ERROR) << m_strDebugInfo << "," << e.what();
			_ASSERT(false);
		}
		catch (...) {
			LOG(ERROR) << m_strDebugInfo;
			_ASSERT(false);
		}
	}
	void SetResult(const T_Result& result)
	{
		m_Result = result;
	}
	/// <summary>
	/// �ڱ�ĵط��Ѿ���m_Result��ֵ
	/// </summary>
	void Run()
	{
		m_Kc.Revert();
		m_hAwaiter.resume();
	}

	bool m_bAwaitReady = false;
private:
	long m_sn;
	T_Result m_Result = {};//�ɵȴ�����ķ���ֵ
	KeepCancel m_Kc;
	std::coroutine_handle<> m_hAwaiter;
	std::string m_strDebugInfo;
};

typedef CoAwaiter<bool> CoAwaiterBool;
//typedef CoTask<uint8_t> CoTaskUint8;
typedef CoTask<bool> CoTaskBool;
struct CoTaskCancel final
{
	CoTaskCancel() :cancel("CoTaskCancel") {

	}
	~CoTaskCancel();
	CoTaskBool co;
	FunCancel��ȫ cancel;
	void TryCancel();
	/// <summary>
	/// ���Э�̻�δ�����Ͳ���ִ���µ�
	/// </summary>
	/// <param name="ref"></param>
	void TryRun(CoTaskBool&& ref)
	{
		if (co.Finished())
		{
			co = std::forward<CoTaskBool&&>(ref);
			co.Run();
		}
	}
};