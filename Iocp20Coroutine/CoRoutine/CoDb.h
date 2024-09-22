#pragma once
#include "CoTask.h"
#include <deque>
#include "../IocpNetwork/Overlapped.h"

template<class T>
class CoDb
{
public:
	void Init(const HANDLE hIocp);
	/// <summary>
	/// 可在主线程协程里调用，实际只把请求放进队列，然后什么也不做
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiter<T>& Save(const T& ref, FunCancel& cancel);
	CoAwaiter<T> Load(FunCancel& cancel);
	/// <summary>
	/// 独立线程调用，很耗时的操作，真的操作数据库，也可能是读写文件，也可能是调用网上的云数据库的接口
	/// </summary>
	void DbThreadProcess();
	CoTask<Iocp::Overlapped::YieldReturn> CoDbDbThreadProcess(Iocp::Overlapped&);
	/// <summary>
	/// 主线程（单线程）调用，得到数据库执行结果（执行协程下一句）
	/// </summary>
	void Process();
	void SaveInDbThread(const T& ref, CoAwaiter<T>& coAwait);
	typedef std::function<void (CoAwaiter<T>&& coAwait)> DbFun;
	std::deque<std::tuple<DbFun, CoAwaiter<T>>> m_dequeSave;
	std::mutex m_mutexDequeSave;

	std::deque<CoAwaiter<T>> m_dequeResult;
	std::mutex m_mutexDequeResult;
	HANDLE m_hIocp = nullptr;
	Iocp::Overlapped m_OverlappedWork;
	Iocp::Overlapped m_OverlappedNotify;
};

