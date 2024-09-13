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
CoAwaiterBool& CoDb<T>::Save(const T& ref, FunCancel& cancel)
{
	std::lock_guard lock(m_mutexDequeSave);

	const auto sn = CoAwaiterBool::GenSn();
	this->m_dequeSave.push_back({ ref, CoAwaiterBool(sn, cancel) });
	CoAwaiterBool& refRet = std::get<1>(this->m_dequeSave.back());
	PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &m_OverlappedNotify.overlapped);
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
	std::deque<std::tuple<T, CoAwaiterBool>> dequeLocal;

	{
		std::lock_guard lock(m_mutexDequeSave);
		while (!m_dequeSave.empty())
		{
			auto&& [ref, coAwait] = this->m_dequeSave.front();
			dequeLocal.push_back({ ref, std::forward<CoAwaiterBool&&>(coAwait) });
			this->m_dequeSave.pop_front();
		}
	}

	while (!dequeLocal.empty())
	{
		auto&& [ref, coAwait] = dequeLocal.front();
		std::ostringstream oss;
		oss << typeid(T).name() << "_" << ref.id << ".bin";
		const auto& strFileName = oss.str();

		// 打开文件
		std::ofstream out(strFileName, std::ios::binary);

		// 检查文件是否成功打开
		if (!out)
		{
			LOG(ERROR) << "无法打开文件" << strFileName;
			dequeLocal.pop_front();
			continue;
		}

		// 写入数据
		out.write(reinterpret_cast<char*>(&ref), sizeof(ref));

		// 关闭文件
		out.close();

		//模拟写硬盘很卡
		std::this_thread::sleep_for(std::chrono::seconds(5));
		{
			std::lock_guard lock(m_mutexDequeResult);
			m_dequeResult.push_back(std::forward<CoAwaiterBool&&>(coAwait));
		}
		dequeLocal.pop_front();
	}
}

template<class T>
inline void CoDb<T>::Process()
{
	std::deque<CoAwaiterBool> dequeLocal;

	{
		std::lock_guard lock(m_mutexDequeResult);
		while (!m_dequeResult.empty())
		{
			auto&& coAwait = m_dequeResult.front();
			dequeLocal.push_back(std::forward<CoAwaiterBool&&>(coAwait));
			this->m_dequeResult.pop_front();
		}
	}

	while (!dequeLocal.empty())
	{
		auto&& coAwait = dequeLocal.front();
		coAwait.Run(true);
		dequeLocal.pop_front();
	}
}
