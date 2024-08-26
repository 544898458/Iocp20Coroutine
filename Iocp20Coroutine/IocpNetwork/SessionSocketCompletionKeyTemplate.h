#include <glog/logging.h>

#include "SessionSocketCompletionKey.h"

//template<class T_Session>
//std::set<Iocp::SessionSocketCompletionKey<T_Session>*> g_setSession;
//template<class T_Session>
//std::mutex g_setSessionMutex;

namespace Iocp {
	template<class T_Session>
	SessionSocketCompletionKey<T_Session>::~SessionSocketCompletionKey()
	{
	}
	//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
	//{
	//	assert(!"SessionSocketCompletionKey不能PostAccept");
	//	return false;
	//}
	template<class T_Session>
	void SessionSocketCompletionKey<T_Session>::StartCoRoutine(HANDLE hIocp)
	{
		m_hIocp = hIocp;
		{
			//auto pOverlapped = new MyOverlapped();
			//PostSend(pOverlapped);
			//pSendOverlapped = new Overlapped();
			sendOverlapped.OnComplete = &Overlapped::OnCompleteSend;
			sendOverlapped.coTask = PostSend(sendOverlapped);
			sendOverlapped.coTask.m_desc = "PostSend";
			PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &notifySendOverlapped.overlapped);
			//sendOverlapped.coTask.Run();
		}
		{
			//pRecvOverlapped = new Overlapped();
			recvOverlapped.coTask = PostRecv(recvOverlapped);
			recvOverlapped.coTask.m_desc = "PostRecv";
			recvOverlapped.coTask.Run();
		}
		return;
	}
	/// <summary>
	/// 必须在主线程（单线程）调用
	/// </summary>
	/// <typeparam name="T_Session"></typeparam>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	template<class T_Session>
	void SessionSocketCompletionKey<T_Session>::Send(const void* buf, int len)
	{
		if (this->sendOverlapped.coTask.Finished())
		{
			LOG(WARNING) << "Sending Failed";
			return;
		}

		this->sendBuf.queue.Enqueue(buf, len);
		//this->sendOverlapped.coTask.Run2(this->sendOverlapped.callSend);
		{
			//std::lock_guard lock(this->sendOverlapped.coTask.m_mutex);
			if (Overlapped::SendState_Sending != this->sendOverlapped.atomicSendState.load())
			{
				PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &notifySendOverlapped.overlapped);
			}
		}
	}
	template<class T_Session>
	bool SessionSocketCompletionKey<T_Session>::Finished()
	{
		std::lock_guard lock(lockFinish);
		return recvFinish && sendFinish;
	}
	template<class T_Session>
	CoTask<Overlapped::YieldReturn> SessionSocketCompletionKey<T_Session>::PostRecv(Overlapped& pOverlapped)
	{
		LOG(INFO) << "调用PostRecv,this=" << this << ",ThreadId=" << GetCurrentThreadId();
		while (true)
		{
			if (!WSARecv(pOverlapped))
			{
				LOG(WARNING) << ("可能断网了,不再调用WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}

			//LOG(INFO) << ("\n准备异步等待WSARecv结果\n");
			co_yield Overlapped::OK;
			//LOG(INFO) << "已异步等到WSARecv结果,numberOfBytesTransferred=" << pOverlapped.numberOfBytesTransferred << ",GetLastErrorReturn=" << pOverlapped.GetLastErrorReturn;
			if (0 == pOverlapped.numberOfBytesTransferred)
			{
				LOG(WARNING) << ("numberOfBytesTransferred==0可能断网了,不再调用WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			const auto [buf, len] = this->recvBuf.Complete(pOverlapped.numberOfBytesTransferred);
			if (len % 1000 == 0)
			{
				LOG(WARNING) << "待处理数据" << len;
			}
			this->recvBuf.PopFront(this->Session.OnRecv(*this, buf, len));//回调用户自定义函数，这里是纯数据，连封包概念都没有，封包是WebSocket协议负责的工作
		}
		{
			std::lock_guard lock(lockFinish);
			recvFinish = true;
			if (!sendFinish)
			{
				LOG(INFO) << "PostRecv协程结束，但是sendOverlapped还没结束,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return Overlapped::Error;
			}
		}
		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostRecv协程结束,GetCurrentThreadId=" << GetCurrentThreadId();
		co_return Overlapped::OK;
	}
	template<class T_Session>
	CoTask<Overlapped::YieldReturn> SessionSocketCompletionKey<T_Session>::PostSend(Overlapped& overlapped)
	{
		while (true)
		{
			bool needYield;
			std::tie(needYield, overlapped.callSend) = WSASend(overlapped);
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
			co_yield overlapped.callSend ? Overlapped::Sending : Overlapped::SendStop;
			//LOG(INFO) << "已异步等到WSASend结果,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
			//	<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len << ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn
			//	<< ",Socket=" << Socket();

			if (!overlapped.callSend)
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
			overlapped.callSend = true;
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

	template<class T_Session>
	bool SessionSocketCompletionKey<T_Session>::WSARecv(Overlapped& refOverlapped)
	{

		DWORD dwRecvCount(0);
		DWORD dwFlag(0);
		refOverlapped.numberOfBytesTransferred = 0;
		std::tie(refOverlapped.wsabuf.buf, refOverlapped.wsabuf.len) = this->recvBuf.BuildRecvBuf();
		//refOverlapped.GetQueuedCompletionStatusReturn
		//LOG(INFO) << ("调用::WSARecv\n");
		const auto recvRet = ::WSARecv(Socket(), &refOverlapped.wsabuf, 1, &dwRecvCount, &dwFlag, &refOverlapped.overlapped, NULL);
		//refOverlapped.GetLastErrorReturn 
		const auto err = WSAGetLastError();

		if (0 == recvRet)//如果未发生任何错误，并且接收操作已立即完成， 则 WSARecv 返回零。 在这种情况下，在调用线程处于可警报状态后，将已计划调用完成例程。
		{
			return true;
		}

		if (SOCKET_ERROR != recvRet ||
			ERROR_IO_PENDING != err)
		{
			switch (err)
			{
			case WSAECONNABORTED:
				LOG(WARNING) << "An established connection was aborted by the software in your host machine.";
				break;
			case WSAENOTSOCK:
				LOG(WARNING) << "An operation was attempted on something that is not a socket.";
				break;

			}
			LOG(WARNING) << "WSARecv重叠的操作未成功启动，并且不会发生完成指示。err=" << err << ",Socket=" << Socket();
			return false;// 任何其他错误代码都指示重叠的操作未成功启动，并且不会发生完成指示。
		}

		// 否则，将返回 值 SOCKET_ERROR ，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。 
		// 错误代码 WSA_IO_PENDING 指示重叠操作已成功启动，稍后将指示完成。 
		return true;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="refOverlapped"></param>
	/// <param name="refSize"></param>
	/// <returns>正常等待异步完成,没有数据要发送没有调用WSASend</returns>
	template<class T_Session>
	std::tuple<bool, bool>  SessionSocketCompletionKey<T_Session>::WSASend(Overlapped& refOverlapped)
	{
		DWORD dwFlag = 0;
		refOverlapped.numberOfBytesTransferred = 0;
		refOverlapped.dwSendCount = 0;
		std::tie(refOverlapped.wsabuf.buf, refOverlapped.wsabuf.len) = this->sendBuf.BuildSendBuf();
		if (nullptr == refOverlapped.wsabuf.buf)
		{
			return std::make_tuple(true, false);
		}
		//refOverlapped.GetQueuedCompletionStatusReturn
		int sendRet = ::WSASend(Socket(), &refOverlapped.wsabuf, 1, &refOverlapped.dwSendCount, dwFlag, &refOverlapped.overlapped, NULL);
		//refOverlapped.GetLastErrorReturn
		int err = WSAGetLastError();
		//refOverlapped.numberOfBytesTransferred = dwSendCount;
		if (0 == sendRet)
		{
			//LOG(INFO) << "WSASend重叠的操作成功启动，已经完成,WSAGetLastError=" << err
			//	<< ",Socket=" << Socket() << ",wsabuf.len=" << refOverlapped.wsabuf.len << ",numberOfBytesTransferred=" << refOverlapped.numberOfBytesTransferred;
			return std::make_tuple(true, true);//如果未发生任何错误，并且发送操作已立即完成， 则 WSASend 返回零。 在这种情况下，一旦调用线程处于可警报状态，就已计划调用完成例程。
		}
		if (SOCKET_ERROR != sendRet ||
			ERROR_IO_PENDING != err)
		{
			LOG(WARNING) << "WSASend重叠的操作未成功启动，并且不会发生完成指示。GetLastErrorReturn =" << refOverlapped.GetLastErrorReturn << ",err=" << err
				<< ",Socket=" << Socket() << ",wsabuf.len=" << refOverlapped.wsabuf.len;
			switch (err)
			{
			case WSAENOTSOCK:
				LOG(WARNING) << "An operation was attempted on something that is not a socket.";
				break;
			case WSAECONNABORTED:
				LOG(WARNING) << "An established connection was aborted by the software in your host machine.";
				break;
			}
			closesocket(Socket());
			return std::make_tuple(false, true);// 任何其他错误代码都指示重叠的操作未成功启动，并且不会发生完成指示。
		}

		// 否则，将返回 值 SOCKET_ERROR ，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。 
		// 错误代码 WSA_IO_PENDING 指示重叠操作已成功启动，稍后将指示完成。 
		//LOG(INFO) << "WSASend重叠的操作成功启动，WSAGetLastError=" << err;
		return std::make_tuple(true, true);
	}
}