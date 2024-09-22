#include "pch.h"
#include "CoDb.h"
#include <deque>
#include <sstream>
#include <fstream>
#include <iostream>



template<class T>
void CoDb<T>::Init(const HANDLE hIocp)
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
void CoDb<T>::LoadFromDbThread(const std::string nickName, CoAwaiter<T>& coAwait)
{
	std::ostringstream oss;
	oss << typeid(T).name() << "_" << nickName << ".bin";
	const auto& strFileName = oss.str();

}

template<class T>
void CoDb<T>::SaveInDbThread(const T& ref, CoAwaiter<T>& coAwait)
{
	std::ostringstream oss;
	oss << typeid(T).name() << "_" << ref.nickName << ".bin";
	const auto& strFileName = oss.str();

	// 打开文件
	std::ofstream out(strFileName, std::ios::binary);

	// 检查文件是否成功打开
	if (!out)
	{
		LOG(ERROR) << "无法打开文件" << strFileName;
		//dequeLocal.pop_front();
		return;
	}

	// 写入数据
	out.write(reinterpret_cast<const char*>(&ref), sizeof(ref));

	// 关闭文件
	out.close();
	LOG(INFO) << "已写入" << strFileName;
	//模拟写硬盘很卡
	std::this_thread::sleep_for(std::chrono::seconds(5));
	{
		coAwait.SetResult(ref);
		std::lock_guard lock(m_mutexDequeResult);
		m_dequeResult.push_back(std::forward<CoAwaiter<T>&&>(coAwait));
	}
};

template<class T>
CoAwaiter<T>& CoDb<T>::Save(const T& ref, FunCancel& cancel)
{
	std::lock_guard lock(m_mutexDequeSave);

	const auto sn = CoAwaiterBool::GenSn();

	this->m_dequeSave.push_back({ [this,&ref](CoAwaiter<T>&& rr) {this->SaveInDbThread(ref,rr); } , CoAwaiter<T>(sn, cancel) });
	CoAwaiter<T>& refRet = std::get<1>(this->m_dequeSave.back());
	if (Iocp::Overlapped::SendState_Sending != this->m_OverlappedNotify.atomicSendState.load())
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &m_OverlappedNotify.overlapped);
	}
	return refRet;
}

template<class T>
CoAwaiter<T> CoDb<T>::Load(FunCancel& cancel)
{

	std::lock_guard lock(m_mutexDequeSave);

	const auto sn = CoAwaiterBool::GenSn();

	this->m_dequeSave.push_back({ [this,&ref](CoAwaiter<T>&& rr) {this->LoadFromDbThread(ref,rr); } , CoAwaiter<T>(sn, cancel) });
	CoAwaiter<T>& refRet = std::get<1>(this->m_dequeSave.back());
	if (Iocp::Overlapped::SendState_Sending != this->m_OverlappedNotify.atomicSendState.load())
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &m_OverlappedNotify.overlapped);
	}
	return refRet;
}
template<class T>
CoTask<Iocp::Overlapped::YieldReturn> CoDb<T>::CoDbDbThreadProcess(Iocp::Overlapped&)
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
void CoDb<T>::DbThreadProcess()
{
	std::deque<std::tuple<DbFun, CoAwaiter<T>>> dequeLocal;

	{
		std::lock_guard lock(m_mutexDequeSave);
		while (!m_dequeSave.empty())
		{
			auto&& [fun, coAwait] = this->m_dequeSave.front();
			dequeLocal.push_back({ fun, std::forward<CoAwaiter<T>&&>(coAwait) });
			this->m_dequeSave.pop_front();
		}
	}

	while (!dequeLocal.empty())
	{
		auto&& [fun, coAwait] = dequeLocal.front();
		//SaveInDbThread(ref, std::forward<CoAwaiterBool&&>(coAwait));
		fun(std::forward<CoAwaiter<T>&&>(coAwait));
		dequeLocal.pop_front();
	}
}

template<class T>
inline void CoDb<T>::Process()
{
	std::deque<CoAwaiter<T>> dequeLocal;

	{
		std::lock_guard lock(m_mutexDequeResult);
		while (!m_dequeResult.empty())
		{
			auto&& coAwait = m_dequeResult.front();
			dequeLocal.push_back(std::forward<CoAwaiter<T>&&>(coAwait));
			this->m_dequeResult.pop_front();
		}
	}

	while (!dequeLocal.empty())
	{
		auto&& coAwait = dequeLocal.front();
		coAwait.Run();
		dequeLocal.pop_front();
	}
}
