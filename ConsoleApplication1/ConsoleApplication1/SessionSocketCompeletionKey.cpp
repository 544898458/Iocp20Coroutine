#include "SessionSocketCompeletionKey.h"
namespace Iocp {
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
			pSendOverlapped = new Overlapped();
			pSendOverlapped->coTask = PostSend(pSendOverlapped);
			pSendOverlapped->coTask.Run();
		}
		{
			pRecvOverlapped = new Overlapped();
			pRecvOverlapped->coTask = PostRecv(pRecvOverlapped);
			pRecvOverlapped->coTask.Run();
		}
		return;
	}
	template<class T_Session>
	void SessionSocketCompeletionKey<T_Session>::Send(const char buf[], int len)
	{
		this->sendBuf.queue.Enqueue(buf, len);
		this->pSendOverlapped->coTask.Run();
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostRecv(Overlapped* pOverlapped)
	{
		while (true)
		{
			if (!WSARecv(pOverlapped))
			{
				printf("可能断网了,不再调用WSARecv\n");
				CloseSocket();
				delete pOverlapped;
				break;
			}

			printf("\n准备异步等待WSARecv结果\n");
			co_yield 0;
			printf("已异步等到WSARecv结果,numberOfBytesTransferred=%d,GetLastErrorReturn=%d\n",
				pOverlapped->numberOfBytesTransferred, pOverlapped->GetLastErrorReturn);
			if (0 == pOverlapped->numberOfBytesTransferred)
			{
				printf("numberOfBytesTransferred==0可能断网了,不再调用WSASend");
				CloseSocket();
				delete pOverlapped;
				break;
			}
			char* buf(nullptr);
			int len(0);
			std::tie(buf, len) = this->recvBuf.Complete(pOverlapped->numberOfBytesTransferred);
			//this->sendBuf.Enqueue("asdf", 5);
			//this->pSendOverlapped->coTask.Run();
			this->recvBuf.PopFront(this->Session.OnRecv(*this, buf, len));
		}
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostSend(Overlapped* pOverlapped)
	{
		while (true)
		{
			bool needYield, callSend;
			std::tie(needYield, callSend) = WSASend(pOverlapped);
			if (!needYield)
			{
				printf("可能断网了,不再调用WSASend");
				delete pOverlapped;
				break;
			}

			printf("准备异步等待WSASend结果\n");
			co_yield 0;
			printf("已异步等到WSASend结果,pOverlapped->numberOfBytesTransferred=%d,callSend=%d\n", pOverlapped->numberOfBytesTransferred, callSend);

			if (!callSend)
			{
				printf("无数据可发，等有数据时再调用WSASend\n");
				continue;
			}

			if (0 == pOverlapped->numberOfBytesTransferred)
			{
				printf("numberOfBytesTransferred==0可能断网了,不再调用WSASend\n");
				CloseSocket();
				delete pOverlapped;
				break;
			}

			this->sendBuf.Complete(pOverlapped->numberOfBytesTransferred);
			co_return 0;
		}
	}

	template<class T_Session>
	bool SessionSocketCompeletionKey<T_Session>::WSARecv(Overlapped* pOverlapped)
	{

		DWORD dwRecvCount(0);
		DWORD dwFlag(0);
		std::tie(pOverlapped->wsabuf.buf, pOverlapped->wsabuf.len) = this->recvBuf.BuildRecvBuf();
		//pOverlapped->GetQueuedCompletionStatusReturn
		const auto recvRet = ::WSARecv(Socket(), &pOverlapped->wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);
		//pOverlapped->GetLastErrorReturn 
		const auto err = WSAGetLastError();

		if (0 == recvRet)//如果未发生任何错误，并且接收操作已立即完成， 则 WSARecv 返回零。 在这种情况下，在调用线程处于可警报状态后，将已计划调用完成例程。
		{
			return true;
		}

		if (SOCKET_ERROR != recvRet ||
			ERROR_IO_PENDING != err)
		{
			printf("WSARecv重叠的操作未成功启动，并且不会发生完成指示。%d\n", err);
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
	std::tuple<bool, bool>  SessionSocketCompeletionKey<T_Session>::WSASend(Overlapped* pOverlapped)
	{
		DWORD dwSendCount(0);
		DWORD dwFlag = 0;
		std::tie(pOverlapped->wsabuf.buf, pOverlapped->wsabuf.len) = this->sendBuf.BuildSendBuf();
		if (nullptr == pOverlapped->wsabuf.buf)
		{
			return std::make_tuple(true, false);
		}
		//pOverlapped->GetQueuedCompletionStatusReturn
		int sendRet = ::WSASend(Socket(), &pOverlapped->wsabuf, 1, &dwSendCount, dwFlag, &pOverlapped->overlapped, NULL);
		//pOverlapped->GetLastErrorReturn
		int err = WSAGetLastError();
		//pOverlapped->numberOfBytesTransferred = dwSendCount;
		if (0 == sendRet)
		{
			return std::make_tuple(true, true);//如果未发生任何错误，并且发送操作已立即完成， 则 WSASend 返回零。 在这种情况下，一旦调用线程处于可警报状态，就已计划调用完成例程。
		}
		if (SOCKET_ERROR != sendRet ||
			ERROR_IO_PENDING != err)
		{
			printf("WSASend重叠的操作未成功启动，并且不会发生完成指示。%d", pOverlapped->GetLastErrorReturn);
			closesocket(Socket());
			return std::make_tuple(false, true);// 任何其他错误代码都指示重叠的操作未成功启动，并且不会发生完成指示。
		}

		// 否则，将返回 值 SOCKET_ERROR ，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。 
		// 错误代码 WSA_IO_PENDING 指示重叠操作已成功启动，稍后将指示完成。 
		return std::make_tuple(true, true);
	}
}