#pragma once
#include "CoTask.h"
#include <deque>
#include "../IocpNetwork/Overlapped.h"

template<class T>
class ������
{
public:
	������():m_OverlappedWork("������"), m_OverlappedNotify("������")
	{

	}
	/// <summary>
	/// ��Ӧ����ɶ˿ڣ�ע�ⲻҪ�������̻߳���һ��
	/// </summary>
	/// <param name="hIocp"></param>
	void Init(const HANDLE hIocp);
	/// <summary>
	/// ���̣߳����̣߳����ã��õ����ݿ�ִ�н����ִ��Э����һ�䣩
	/// </summary>
	void Process();

	using SpCoAwaiterT = std::shared_ptr<CoAwaiter<T>>;
	using DbFun = std::function<T()>;
	CoAwaiter<T>& DoDb(DbFun funDb, FunCancel& cancel);
	CoTask<Iocp::Overlapped::YieldReturn> CoDbDbThreadProcess(Iocp::Overlapped&);
	

	/// <summary>
	/// �����̵߳��ã��ܺ�ʱ�Ĳ��������ͬ���������ݿ⣬Ҳ�����Ƕ�д�ļ���Ҳ�����ǵ������ϵ������ݿ�Ľӿ�
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
	/// �������߳�Э������ã�ʵ��ֻ������Ž����У�Ȼ��ʲôҲ����
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiter<T>& CoSave(const T& ref, const std::string& strNickName, FunCancel& cancel);
	CoAwaiter<T>& Load(const std::string nickName, FunCancel& cancel);
	������<T> m_������;
private:
	T SaveInDbThread(const T& ref, const std::string& strNickName);
	T LoadFromDbThread(const std::string nickName);
};
