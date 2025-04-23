#pragma once
#include "CoTask.h"
#include <deque>
#include "../IocpNetwork/Overlapped.h"

template<class T>
class 慢操作
{
public:
	慢操作():m_OverlappedWork("慢操作"), m_OverlappedNotify("慢操作")
	{

	}
	/// <summary>
	/// 对应到完成端口，注意不要和网络线程混在一起
	/// </summary>
	/// <param name="hIocp"></param>
	void Init(const HANDLE hIocp);
	/// <summary>
	/// 主线程（单线程）调用，得到数据库执行结果（执行协程下一句）
	/// </summary>
	void Process();

	using SpCoAwaiterT = std::shared_ptr<CoAwaiter<T>>;
	using DbFun = std::function<T()>;
	CoAwaiter<T>& DoDb(DbFun funDb, FunCancel& cancel);
	CoTask<Iocp::Overlapped::YieldReturn> CoDbDbThreadProcess(Iocp::Overlapped&);
	

	/// <summary>
	/// 独立线程调用，很耗时的操作，真的同步操作数据库，也可能是读写文件，也可能是调用网上的云数据库的接口
	/// </summary>
	void DbThreadProcess();
private:
	std::deque<std::tuple<DbFun, SpCoAwaiterT>> m_dequeSave;
	std::mutex m_mutexDequeSave;

	std::deque<SpCoAwaiterT> m_dequeResult;
	std::mutex m_mutexDequeResult;
	HANDLE m_hIocp = nullptr;
	Iocp::Overlapped m_OverlappedWork;
	Iocp::Overlapped m_OverlappedNotify;

};

template<class T>
class CoDb
{
public:
	/// <summary>
	/// 可在主线程协程里调用，实际只把请求放进队列，然后什么也不做
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiter<T>& CoSave(const T& ref, const std::string& strNickName, FunCancel& cancel);
	CoAwaiter<T>& Load(const std::string nickName, FunCancel& cancel);
	慢操作<T> m_慢操作;
private:
	T SaveInDbThread(const T& ref, const std::string& strNickName);
	T LoadFromDbThread(const std::string nickName);
};
