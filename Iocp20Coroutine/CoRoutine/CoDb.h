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
	/// �������߳�Э������ã�ʵ��ֻ������Ž����У�Ȼ��ʲôҲ����
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiter<T>& Save(const T& ref, FunCancel& cancel);
	CoAwaiter<T>& Load(const std::string nickName, FunCancel& cancel);
	/// <summary>
	/// �����̵߳��ã��ܺ�ʱ�Ĳ�������Ĳ������ݿ⣬Ҳ�����Ƕ�д�ļ���Ҳ�����ǵ������ϵ������ݿ�Ľӿ�
	/// </summary>
	void DbThreadProcess();
	CoTask<Iocp::Overlapped::YieldReturn> CoDbDbThreadProcess(Iocp::Overlapped&);
	/// <summary>
	/// ���̣߳����̣߳����ã��õ����ݿ�ִ�н����ִ��Э����һ�䣩
	/// </summary>
	void Process();
	using SpCoAwaiterT = std::shared_ptr<CoAwaiter<T>>;
	void SaveInDbThread(const T& ref, SpCoAwaiterT& spCoAwaiter);
	void LoadFromDbThread(const std::string nickName, SpCoAwaiterT& spCoAwait);
	typedef std::function<void (SpCoAwaiterT& sp)> DbFun;
	std::deque<std::tuple<DbFun, SpCoAwaiterT>> m_dequeSave;
	std::mutex m_mutexDequeSave;
	
	std::deque<SpCoAwaiterT> m_dequeResult;
	std::mutex m_mutexDequeResult;
	HANDLE m_hIocp = nullptr;
	Iocp::Overlapped m_OverlappedWork;
	Iocp::Overlapped m_OverlappedNotify;
private:
	CoAwaiter<T>& DoDb(DbFun funDb, FunCancel& cancel);
};

