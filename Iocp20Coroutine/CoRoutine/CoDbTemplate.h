#include "pch.h"
#include "CoDb.h"
#include <deque>
#include <sstream>
#include <fstream>
#include <iostream>



template<class T>
inline void CoDb<T>::Init(const HANDLE hIocp)
{
	m_hIocp = hIocp;
	m_OverlappedNotify.OnComplete = &Iocp::Overlapped::OnCompleteNotifySend;
	m_OverlappedNotify.pOverlapped = &m_OverlappedWork;
	m_OverlappedNotify.coTask.m_desc = "NotifyDb";

	m_OverlappedWork.OnComplete = &Iocp::Overlapped::OnCompleteSend;
	m_OverlappedWork.coTask.m_desc = "DbWork";
	m_OverlappedWork.coTask = CoDbDbThreadProcess(sendOverlapped);
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

CoTask<int> CoDbDbThreadProcess(Iocp::Overlapped&)
{
	{
		while (true)
		{
			bool needYield(false), callSend(false);
			std::tie(needYield, callSend) = WSASend(overlapped);
			if (!needYield)
			{
				LOG(INFO) << ("可能断网了,不再调用WSASend");
				//delete pOverlapped;
				break;
			}
			//if (overlapped.callSend)
			//	LOG(INFO) << "准备异步等待WSASend结果";
			//else
			//	LOG(INFO) << "等有数据再发WSASend" ;

			//LOG(INFO) << "开始异步等WSASend结果,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
				//<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len
				//<< ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn;
			co_yield callSend ? Overlapped::Sending : Overlapped::SendStop;
			//LOG(INFO) << "已异步等到WSASend结果,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
			//	<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len << ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn
			//	<< ",Socket=" << Socket();

			if (!callSend)
			{
				//LOG(INFO) << ("有数据了，准备发WSASend\n");
				continue;
			}


			if (0 == overlapped.numberOfBytesTransferred && overlapped.GetLastErrorReturn != ERROR_IO_PENDING)
			{
				LOG(WARNING) << ("numberOfBytesTransferred==0可能断网了,不再调用WSASend,pOverlapped.GetLastErrorReturn=%d,GetThreadId=%d\n", overlapped.GetLastErrorReturn, GetCurrentThreadId());
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			//if (pOverlapped.GetLastErrorReturn == ERROR_IO_PENDING) 
			//{
			//	printf("ERROR_IO_PENDING还没发完，下次还要接着发\n");
			//}

			this->sendBuf.Complete(overlapped.numberOfBytesTransferred);
			//overlapped.callSend = true;
		}

		//if (!this->recvOverlapped.coTask.Finished())

		{
			std::lock_guard lock(lockFinish);
			sendFinish = true;
			if (!recvFinish)
			{
				LOG(INFO) << "PostSend协程结束，但是recvOverlapped还没结束,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return Overlapped::Error;
			}
		}

		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostSend协程结束,GetCurrentThreadId=" << GetCurrentThreadId();
		co_return Overlapped::OK;
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
		std::this_thread::sleep_for(std::chrono::seconds(1));
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
