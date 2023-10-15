#pragma once
#include "SocketCompeletionKey.h"
#include<WinSock2.h>
#include "ByteQueue.h"
namespace Iocp {
	template<class T_Session>
	class SessionSocketCompeletionKey :public SocketCompeletionKey
	{
	public:
		SessionSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
		{

		}
		virtual void StartCoRoutine() override;
		void Send(const char buf[], int len);
	private:
		CoTask<int> PostSend(Overlapped* pOverlapped);
		CoTask<int> PostRecv(Overlapped* pOverlapped);
		std::tuple<bool, bool>  WSASend(Overlapped* pOverlapped);
		bool WSARecv(Overlapped* pOverlapped);

	private:
		T_Session Session;
		ByteQueueSend sendBuf;
		ByteQueueRecv recvBuf;
		Overlapped* pSendOverlapped;
		Overlapped* pRecvOverlapped;
	};
}