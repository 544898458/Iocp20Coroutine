#pragma once
#include "CoTask.h"
#include <deque>
#include "../IocpNetwork/Overlapped.h"

template<class T>
class CoDb
{
public:
	/// <summary>
	/// 对应到完成端口，注意不要和网络线程混在一起
	/// </summary>
	/// <param name="hIocp"></param>
	void Init(const HANDLE hIocp);
	/// <summary>
	/// 可在主线程协程里调用，实际只把请求放进队列，然后什么也不做
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiterBool& Save(const T& ref, FunCancel& cancel);
	CoAwaiter<T>& Load(const std::string nickName, FunCancel& cancel);
	/// <summary>
	/// 主线程（单线程）调用，得到数据库执行结果（执行协程下一句）
	/// </summary>
	void Process();

private:
	using SpCoAwaiterT = std::shared_ptr<CoAwaiter<T>>;
	void SaveInDbThread(const T& ref, SpCoAwaiterT& spCoAwaiter);
	void LoadFromDbThread(const std::string nickName, SpCoAwaiterT& spCoAwait);
	CoTask<Iocp::Overlapped::YieldReturn> CoDbDbThreadProcess(Iocp::Overlapped&);
	CoAwaiter<T>& DoDb(DbFun funDb, FunCancel& cancel);
	/// <summary>
	/// 独立线程调用，很耗时的操作，真的同步操作数据库，也可能是读写文件，也可能是调用网上的云数据库的接口
	/// </summary>
	void DbThreadProcess();
	typedef std::function<void(SpCoAwaiterT& sp)> DbFun;
	std::deque<std::tuple<DbFun, SpCoAwaiterT>> m_dequeSave;
	std::mutex m_mutexDequeSave;

	std::deque<SpCoAwaiterT> m_dequeResult;
	std::mutex m_mutexDequeResult;
	HANDLE m_hIocp = nullptr;
	Iocp::Overlapped m_OverlappedWork;
	Iocp::Overlapped m_OverlappedNotify;
};

