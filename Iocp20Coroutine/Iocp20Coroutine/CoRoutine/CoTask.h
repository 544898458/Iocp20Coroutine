#pragma once
#include<glog/logging.h>
#include<coroutine>
#include<chrono>
#include<assert.h>
#include<mutex>
#include<functional>
using namespace std;

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
	struct promise_type {
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

		/// <summary>
		/// Э�̽���ǰִ��
		/// ����ĳ�suspend_never������ܶ࣬��֪��Ϊʲô
		/// </summary>
		/// <returns></returns>
		auto final_suspend() noexcept
		{
			return std::suspend_always{};
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

	const std::chrono::duration<int, std::milli> length = 1000ms;
	using handle = std::coroutine_handle<promise_type>;
	/// <summary>
	/// ����ִ��Э�̺�������ֱִ������yield����
	/// </summary>
	void Run()
	{
		std::lock_guard lock(m_mutex);
		if (FinishedNoLock())
		{
			LOG(INFO) << m_desc << "Э���ѽ���������ִ��\n";
			return;
		}
		if (m_hCoroutine == nullptr)
			return;
		m_hCoroutine.resume();
		TryClear();
	}
	/// <summary>
	/// Sendר��
	/// </summary>
	/// <param name="send"></param>
	void Run2(const bool& send)
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
			LOG(INFO) << m_desc << "Э�����˳�" << m_hCoroutine.address();
			m_hCoroutine.destroy();
			m_hCoroutine = nullptr;
		}
	}
	bool Finished()
	{
		std::lock_guard lock(m_mutex);
		return FinishedNoLock();
	}
	bool FinishedNoLock()
	{
		if (m_hCoroutine == nullptr || m_hCoroutine.address() == nullptr)
			return true;

		return m_hCoroutine.done();
	}
private:
	std::mutex m_mutex;
	/// <summary>
	/// Ψһ�ĳ�Ա����
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
	CoTask(CoTask&& other)noexcept :m_hCoroutine(other.m_hCoroutine) { other.m_hCoroutine = nullptr; }

	/// <summary>
	/// �ƶ����壬���Ǳ���ģ�Э�̷��ص���ʱ��������ֵ���ã���ͳ���ƹ��캯�������const&�޷��޸Ĵ������Ķ���
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
	KeepCancel(std::function<void()>& old,bool autoRevert=true) :funCancelOld(old), refFunCancel(old), m_autoRevert(autoRevert)
	{
	}
	~KeepCancel() 
	{
		if (m_autoRevert)
			Revert();
	}
	void Revert()
	{
		LOG(INFO) << "Revert" ;
		refFunCancel = funCancelOld;
		funCancelOld = nullptr;
	}
	std::function<void()> funCancelOld;
	std::function<void()>& refFunCancel;
	void operator=(const KeepCancel& other) 
	{
		funCancelOld = refFunCancel = other.refFunCancel;
		m_autoRevert = other.m_autoRevert;
	}
	bool m_autoRevert;
};


struct CoAwaiter
{
	static std::function<void()> funEmpty;
	CoAwaiter(bool initSn = false, std::function<void()>& cancel = funEmpty) :m_Canceled(false), m_Kc(cancel,false)
	{
		if (initSn)
			m_sn = GenSn();
	}

	static long GenSn()
	{
		static long sn = 0;
		return ++sn;
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
	bool await_resume() const noexcept
	{
		return m_Canceled;
	}
	constexpr bool await_ready() const noexcept { return false; }
	constexpr void await_suspend(std::coroutine_handle<> h) noexcept
	{
		//auto t = std::jthread([h, l = length] {
		//	std::this_thread::sleep_for(l);
		//h.resume();
		//	});
		m_hAwaiter = h;
	}
	
	CoAwaiter(const CoAwaiter& other):m_Kc(other.m_Kc.refFunCancel,false)
	{
		CopyFrom(other);
	}
	void CopyFrom(const CoAwaiter& other)
	{
		m_hAwaiter = other.m_hAwaiter;
		m_sn = other.m_sn;
		//other.m_hAwaiter = nullptr;
		//other.m_sn = 0;
		m_Canceled = other.m_Canceled;
		m_Kc = other.m_Kc;
	}
	CoAwaiter(CoAwaiter&& other) noexcept :m_Kc(other.m_Kc.refFunCancel, false)
	{
		MoveFrom(other);
	}
	void operator =(CoAwaiter&& other)noexcept
	{
		MoveFrom(other);
	}
	void operator =(const CoAwaiter& other)noexcept
	{
		CopyFrom(other);
	}
	void MoveFrom(CoAwaiter& other)noexcept
	{
		m_hAwaiter = other.m_hAwaiter;
		m_sn = other.m_sn;
		other.m_hAwaiter = nullptr;
		other.m_sn = 0;
		m_Canceled = other.m_Canceled;
	}
	void Cancel()
	{
		m_Canceled = true;
		m_hAwaiter.resume();
	}
	void Run() 
	{
		m_hAwaiter.resume();
	}
private:
	long m_sn;
	bool m_Canceled;
	KeepCancel m_Kc;
	std::coroutine_handle<> m_hAwaiter;
};
