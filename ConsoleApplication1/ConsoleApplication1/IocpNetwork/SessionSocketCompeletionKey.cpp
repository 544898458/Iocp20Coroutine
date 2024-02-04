#include <glog/logging.h>

#include "SessionSocketCompeletionKey.h"
namespace Iocp {
	template<class T_Session>
	SessionSocketCompeletionKey<T_Session>::~SessionSocketCompeletionKey()
	{
	}
	//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
	//{
	//	assert(!"SessionSocketCompeletionKey不能PostAccept");
	//	return false;
	//}
	template<class T_Session>
	void SessionSocketCompeletionKey<T_Session>::StartCoRoutine()
	{
		{
			//auto pOverlapped = new MyOverlapped();
			//PostSend(pOverlapped);
			//pSendOverlapped = new Overlapped();
			sendOverlapped.coTask = PostSend(sendOverlapped);
			sendOverlapped.coTask.desc = "PostSend";
			sendOverlapped.coTask.Run();
		}
		{
			//pRecvOverlapped = new Overlapped();
			recvOverlapped.coTask = PostRecv(recvOverlapped);
			recvOverlapped.coTask.desc = "PostRecv";
			recvOverlapped.coTask.Run();
		}
		return;
	}
	template<class T_Session>
	void SessionSocketCompeletionKey<T_Session>::Send(const char buf[], int len)
	{
		if (this->sendOverlapped.coTask.Finished())
			return;

		this->sendBuf.queue.Enqueue(buf, len);
		this->sendOverlapped.coTask.Run2(this->sendOverlapped.callSend);
	}
	template<class T_Session>
	bool SessionSocketCompeletionKey<T_Session>::Finished()
	{
		std::lock_guard lock(lockFinish);
		return recvFinish && sendFinish;
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostRecv(Overlapped& pOverlapped)
	{
		LOG(INFO) << "调用PostRecv,this=" << this << ",ThreadId=" << GetCurrentThreadId();
		while (true)
		{
			if (!WSARecv(pOverlapped))
			{
				LOG(INFO) << ("可能断网了,不再调用WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}

			LOG(INFO) << ("\n准备异步等待WSARecv结果\n");
			co_yield 0;
			LOG(INFO) << "已异步等到WSARecv结果,numberOfBytesTransferred=" << pOverlapped.numberOfBytesTransferred << ",GetLastErrorReturn=" << pOverlapped.GetLastErrorReturn;
			if (0 == pOverlapped.numberOfBytesTransferred)
			{
				LOG(INFO) << ("numberOfBytesTransferred==0可能断网了,不再调用WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			char* buf(nullptr);
			int len(0);
			std::tie(buf, len) = this->recvBuf.Complete(pOverlapped.numberOfBytesTransferred);
			//this->sendBuf.Enqueue("asdf", 5);
			//this->pSendOverlapped->coTask.Run();
			this->recvBuf.PopFront(this->Session.OnRecv(*this, buf, len));//回调用户自定义函数，这里是纯数据，连封包概念都没有，封包是WebSocket协议负责的内容
		}
		//this->pSendOverlapped.coTask.Run();
		//if (!this->sendOverlapped.coTask.Finished())
		{
			std::lock_guard lock(lockFinish);
			recvFinish = true;
			if (!sendFinish)
			{
				LOG(INFO) << "PostRecv协程结束，但是sendOverlapped还没结束,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return;
			}
		}
		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostRecv协程结束,GetCurrentThreadId=" << GetCurrentThreadId();
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostSend(Overlapped& overlapped)
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
			if (overlapped.callSend)
				LOG(INFO) << ("准备异步等待WSASend结果,GetThreadId=%d\n", GetCurrentThreadId());
			else
				LOG(INFO) << ("等有数据再发WSASend,GetThreadId=%d\n", GetCurrentThreadId());

			LOG(INFO) << "开始异步等WSASend结果,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
				<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len
				<< ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn << ",GetThreadId=GetCurrentThreadId()";
			co_yield 0;
			LOG(INFO) << "已异步等到WSASend结果,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
				<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len << ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn
				<< ",GetThreadId=" << GetCurrentThreadId();

			if (!overlapped.callSend)
			{
				LOG(INFO) << ("有数据了，准备发WSASend\n");
				continue;
			}


			if (0 == overlapped.numberOfBytesTransferred && overlapped.GetLastErrorReturn != ERROR_IO_PENDING)
			{
				LOG(INFO) << ("numberOfBytesTransferred==0可能断网了,不再调用WSASend,pOverlapped.GetLastErrorReturn=%d,GetThreadId=%d\n", overlapped.GetLastErrorReturn, GetCurrentThreadId());
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			//if (pOverlapped.GetLastErrorReturn == ERROR_IO_PENDING) 
			//{
			//	printf("ERROR_IO_PENDING还没发完，下次还要接着发\n");
			//}

			this->sendBuf.Complete(overlapped.numberOfBytesTransferred);
		}

		//if (!this->recvOverlapped.coTask.Finished())

		{
			std::lock_guard lock(lockFinish);
			sendFinish = true;
			if (!recvFinish)
			{
				LOG(INFO) << "PostSend协程结束，但是recvOverlapped还没结束,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return;
			}
		}

		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostSend协程结束,GetCurrentThreadId=" << GetCurrentThreadId();
	}

	template<class T_Session>
	bool SessionSocketCompeletionKey<T_Session>::WSARecv(Overlapped& pOverlapped)
	{

		DWORD dwRecvCount(0);
		DWORD dwFlag(0);
		std::tie(pOverlapped.wsabuf.buf, pOverlapped.wsabuf.len) = this->recvBuf.BuildRecvBuf();
		//pOverlapped.GetQueuedCompletionStatusReturn
		LOG(INFO) << ("调用::WSARecv\n");
		const auto recvRet = ::WSARecv(Socket(), &pOverlapped.wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped.overlapped, NULL);
		//pOverlapped.GetLastErrorReturn 
		const auto err = WSAGetLastError();

		if (0 == recvRet)//如果未发生任何错误，并且接收操作已立即完成， 则 WSARecv 返回零。 在这种情况下，在调用线程处于可警报状态后，将已计划调用完成例程。
		{
			return true;
		}

		if (SOCKET_ERROR != recvRet ||
			ERROR_IO_PENDING != err)
		{
			LOG(INFO) << "WSARecv重叠的操作未成功启动，并且不会发生完成指示。" << err;
			return false;// 任何其他错误代码都指示重叠的操作未成功启动，并且不会发生完成指示。
		}

		// 否则，将返回 值 SOCKET_ERROR ，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。 
		// 错误代码 WSA_IO_PENDING 指示重叠操作已成功启动，稍后将指示完成。 
		return true;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pOverlapped"></param>
	/// <param name="refSize"></param>
	/// <returns>正常等待异步完成,没有数据要发送没有调用WSASend</returns>
	template<class T_Session>
	std::tuple<bool, bool>  SessionSocketCompeletionKey<T_Session>::WSASend(Overlapped& pOverlapped)
	{
		DWORD dwFlag = 0;
		pOverlapped.numberOfBytesTransferred = 0;
		pOverlapped.dwSendCount = 0;
		std::tie(pOverlapped.wsabuf.buf, pOverlapped.wsabuf.len) = this->sendBuf.BuildSendBuf();
		if (nullptr == pOverlapped.wsabuf.buf)
		{
			return std::make_tuple(true, false);
		}
		//pOverlapped.GetQueuedCompletionStatusReturn
		int sendRet = ::WSASend(Socket(), &pOverlapped.wsabuf, 1, &pOverlapped.dwSendCount, dwFlag, &pOverlapped.overlapped, NULL);
		//pOverlapped.GetLastErrorReturn
		int err = WSAGetLastError();
		//pOverlapped.numberOfBytesTransferred = dwSendCount;
		if (0 == sendRet)
		{
			LOG(INFO) << "WSASend重叠的操作成功启动，WSAGetLastError=" << err;
			return std::make_tuple(true, true);//如果未发生任何错误，并且发送操作已立即完成， 则 WSASend 返回零。 在这种情况下，一旦调用线程处于可警报状态，就已计划调用完成例程。
		}
		if (SOCKET_ERROR != sendRet ||
			ERROR_IO_PENDING != err)
		{
			LOG(INFO) << "WSASend重叠的操作未成功启动，并且不会发生完成指示。" << pOverlapped.GetLastErrorReturn;
			closesocket(Socket());
			return std::make_tuple(false, true);// 任何其他错误代码都指示重叠的操作未成功启动，并且不会发生完成指示。
		}

		// 否则，将返回 值 SOCKET_ERROR ，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。 
		// 错误代码 WSA_IO_PENDING 指示重叠操作已成功启动，稍后将指示完成。 
		return std::make_tuple(true, true);
	}
}