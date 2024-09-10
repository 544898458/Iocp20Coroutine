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
				LOG(INFO) << ("���ܶ�����,���ٵ���WSASend");
				//delete pOverlapped;
				break;
			}
			//if (overlapped.callSend)
			//	LOG(INFO) << "׼���첽�ȴ�WSASend���";
			//else
			//	LOG(INFO) << "���������ٷ�WSASend" ;

			//LOG(INFO) << "��ʼ�첽��WSASend���,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
				//<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len
				//<< ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn;
			co_yield callSend ? Overlapped::Sending : Overlapped::SendStop;
			//LOG(INFO) << "���첽�ȵ�WSASend���,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
			//	<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len << ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn
			//	<< ",Socket=" << Socket();

			if (!callSend)
			{
				//LOG(INFO) << ("�������ˣ�׼����WSASend\n");
				continue;
			}


			if (0 == overlapped.numberOfBytesTransferred && overlapped.GetLastErrorReturn != ERROR_IO_PENDING)
			{
				LOG(WARNING) << ("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSASend,pOverlapped.GetLastErrorReturn=%d,GetThreadId=%d\n", overlapped.GetLastErrorReturn, GetCurrentThreadId());
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			//if (pOverlapped.GetLastErrorReturn == ERROR_IO_PENDING) 
			//{
			//	printf("ERROR_IO_PENDING��û���꣬�´λ�Ҫ���ŷ�\n");
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
				LOG(INFO) << "PostSendЭ�̽���������recvOverlapped��û����,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return Overlapped::Error;
			}
		}

		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostSendЭ�̽���,GetCurrentThreadId=" << GetCurrentThreadId();
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

		// ���ļ�
		std::ofstream out(strFileName, std::ios::binary);

		// ����ļ��Ƿ�ɹ���
		if (!out)
		{
			LOG(ERROR) << "�޷����ļ�" << strFileName;
			dequeLocal.pop_front();
			continue;
		}

		// д������
		out.write(reinterpret_cast<char*>(&ref), sizeof(ref));

		// �ر��ļ�
		out.close();

		//ģ��дӲ�̺ܿ�
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
