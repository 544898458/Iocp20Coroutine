#include <vld.h>
#include <glog/logging.h>
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include <thread>
#include"./CoRoutine/CoTask.h"

static int t = 1;
//#define T 9  

using namespace std;

typedef int Status;
CoTask<int> PostSend()
{
	co_return 0;
	//LOG(INFO) << "PostSend";
	//bool needYield, callSend;
	//co_yield 0;
	//LOG(INFO) << ("PostSend end\n");
}
void NetworkThreadProc(CoTask<int>* m_coWalk)
{
	LOG(INFO) << ("NetworkThreadProc\n");
	m_coWalk->Run();
}


#include <iostream>
#include<coroutine>

using namespace std;

template<typename T>
class MyCoroutine
{
public:
	// Э�̿�ʼʱ����Э�̵�״̬��������ڴ�󣬵���promise_type�Ĺ��캯��
	struct promise_type {
		T value;
		// ΪЭ�̵�״̬��������ڴ�ʧ��ʱ
		static auto get_return_object_on_allocation_failure() { return MyCoroutine{ nullptr }; }

		// ����ɹ���ʼִ��
		auto get_return_object() { return MyCoroutine{ handle::from_promise(*this) }; }
		// �����Ϻ�����ִ��
		auto initial_suspend() { return std::suspend_always{}; }
		// Э�̽���ǰִ��
		auto final_suspend() noexcept { return std::suspend_always{}; }
		// ����δ��������쳣ʱִ��
		void unhandled_exception() { return std::terminate(); }
		// co_return ʱִ�У�return_void��return_value��ѡһ
		void return_void() {}
		//int return_value(int result) { this.result = reslut; }

		//co_yieldʱִ��
		auto yield_value(T value) { this->value = value; return std::suspend_always{}; }

	};
	using handle = std::coroutine_handle<promise_type>;
private:
	handle hCoroutine;
	MyCoroutine(handle handle) :hCoroutine(handle) {}
public:
	//int result;
	MyCoroutine(MyCoroutine&& other)noexcept :hCoroutine(other.hCoroutine) { other.hCoroutine = nullptr; }
	~MyCoroutine() { if (hCoroutine) hCoroutine.destroy(); }
	bool MoveNext() const { return hCoroutine && (hCoroutine.resume(), !hCoroutine.done()); }
	T GetValue() const { return hCoroutine.promise().value; }
};

MyCoroutine<int> task()
{
	int a = 0, b = 1;
	while (a <= 10)
	{
		co_yield a;
		a++;
	}
};

//int main()
//{
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
//	MyCoroutine<int> fun = task();
//	while (fun.MoveNext())
//	{
//		cout << fun.GetValue() << endl;
//		//getchar();
//	}
//	_CrtDumpMemoryLeaks();	 //��ʾ�ڴ�й©����
//}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
	Status i = 1;
	//LOG(INFO) << T * i << endl; //Test Cout  
	CoTask<int> m_coWalk = PostSend();
	//std::thread networkThread(NetworkThreadProc, &m_coWalk);
	//networkThread.detach();
	//std::this_thread::sleep_for(2000ms);
	//std::thread networkThread2(NetworkThreadProc, &m_coWalk);
	//networkThread2.detach();
	//std::this_thread::sleep_for(2000ms);
	m_coWalk.Run();
	_CrtDumpMemoryLeaks();	 //��ʾ�ڴ�й©����
	return 0;
}
