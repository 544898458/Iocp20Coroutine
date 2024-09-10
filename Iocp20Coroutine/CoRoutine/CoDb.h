#pragma once
#include "CoTask.h"
#include <deque>
struct Overlapped_DB
{
	Overlapped_DB()// :coAwait(0, funCancel), OnComplete(&Overlapped::OnCompleteNormal) {}
	{

	}
	//enum Op
	//{
	//	Accept,
	//	Recv,
	//	Send,
	//};

	OVERLAPPED overlapped = { 0 };
};

template<class T>
class CoDb
{
public:
	void Init(const HANDLE hIocp) 
	{
		m_hIocp = hIocp;
	}
	/// <summary>
	/// �������߳�Э������ã�ʵ��ֻ������Ž����У�Ȼ��ʲôҲ����
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiterBool& Save(const T& ref, FunCancel& cancel);
	/// <summary>
	/// �����̵߳��ã��ܺ�ʱ�Ĳ�������Ĳ������ݿ⣬Ҳ�����Ƕ�д�ļ���Ҳ�����ǵ������ϵ������ݿ�Ľӿ�
	/// </summary>
	void DbThreadProcess();
	/// <summary>
	/// ���̣߳����̣߳����ã��õ����ݿ�ִ�н����ִ��Э����һ�䣩
	/// </summary>
	void Process();
	std::deque<std::tuple<T, CoAwaiterBool>> m_dequeSave;
	std::mutex m_mutexDequeSave;

	std::deque<CoAwaiterBool> m_dequeResult;
	std::mutex m_mutexDequeResult;
	HANDLE m_hIocp = nullptr;
	Overlapped_DB m_Overlapped;
};

