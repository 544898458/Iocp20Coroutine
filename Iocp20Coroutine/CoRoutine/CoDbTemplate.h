#include "CoDb.h"
#include <deque>
#include <sstream>
#include "../IocpNetwork/MsgPack.h"
#include "../读配置文件/文件存取对象.h"


template<class T>
void 慢操作<T>::Init(const HANDLE hIocp)
{
	m_hIocp = hIocp;
	m_OverlappedNotify.OnComplete = &Iocp::Overlapped::OnCompleteNotifySend;
	m_OverlappedNotify.pOverlapped = &m_OverlappedWork;
	m_OverlappedNotify.coTask.m_desc = "NotifyDb";

	m_OverlappedWork.OnComplete = &Iocp::Overlapped::OnCompleteSend;
	m_OverlappedWork.coTask.m_desc = "DbWork";
	m_OverlappedWork.coTask = CoDbDbThreadProcess(m_OverlappedWork);
	m_OverlappedWork.coTask.Run();
}

template<class T>
T CoDb<T>::LoadFromDbThread(const std::string nickName) 
{
	return 从文件里读出对象<T>(nickName);
}

template<class T>
T CoDb<T>::SaveInDbThread(const T& ref, const std::string& strNickName)
{
	return 写对象进文件(ref, strNickName);
}

template<class T>
CoAwaiter<T>& CoDb<T>::CoSave(const T& ref, const std::string& strNickName, FunCancel& cancel)
{
	return m_慢操作.DoDb([this, &ref, strNickName]() {return this->SaveInDbThread(ref, strNickName); }, cancel);
}

template<class T>
CoAwaiter<T>& 慢操作<T>::DoDb(DbFun funDb, FunCancel& cancel)
{
	std::lock_guard lock(m_mutexDequeSave);

	const auto sn = CoAwaiterBool::GenSn();

	auto sp = std::make_shared<CoAwaiter<T>, const long&, FunCancel&, const std::string&>(sn, cancel, "");
	this->m_dequeSave.push_back({ funDb, sp});
	if (Iocp::Overlapped::SendState_Sending != this->m_OverlappedNotify.atomicSendState.load())
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &m_OverlappedNotify.overlapped);
	}

	return *sp;
}

template<class T>
CoAwaiter<T>& CoDb<T>::Load(const std::string nickName, FunCancel& cancel)
{
	return m_慢操作.DoDb([this, nickName]() { return this->LoadFromDbThread(nickName); }, cancel);
}

template<class T>
CoTask<Iocp::Overlapped::YieldReturn> 慢操作<T>::CoDbDbThreadProcess(Iocp::Overlapped&)
{
	{
		while (true)
		{
			DbThreadProcess();
			co_yield Iocp::Overlapped::SendStop;
		}

		co_return Iocp::Overlapped::OK;
	}
}
template<class T>
void 慢操作<T>::DbThreadProcess()
{
	std::deque<std::tuple<DbFun, SpCoAwaiterT>> dequeLocal;

	{
		std::lock_guard lock(m_mutexDequeSave);
		while (!m_dequeSave.empty())
		{
			auto& [fun, spCoAwait] = this->m_dequeSave.front();
			dequeLocal.push_back({ fun, spCoAwait });
			this->m_dequeSave.pop_front();
			LOG(INFO) << "m_dequeSave.size:" << this->m_dequeSave.size();
		}
	}

	while (!dequeLocal.empty())
	{
		auto& [fun, spCoAwait] = dequeLocal.front();
		//SaveInDbThread(ref, std::forward<CoAwaiterBool&&>(coAwait));
		//模拟读硬盘很卡
		spCoAwait->SetResult(fun());
		m_dequeResult.push_back(spCoAwait);
		dequeLocal.pop_front();
	}
}

template<class T>
inline void 慢操作<T>::Process()
{
	{
		std::lock_guard lock(m_mutexDequeResult);
		while (!m_dequeResult.empty())
		{
			auto& spCoAwait = m_dequeResult.front();
			spCoAwait->Run();
			this->m_dequeResult.pop_front();
		}
	}
}
