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
		virtual ~SessionSocketCompeletionKey();
		virtual void StartCoRoutine() override;
		void Send(const char buf[], int len);
		T_Session Session;

	private:
		CoTask<int> PostSend(Overlapped& pOverlapped);
		CoTask<int> PostRecv(Overlapped& pOverlapped);
		std::tuple<bool, bool>  WSASend(Overlapped& pOverlapped);
		bool WSARecv(Overlapped& pOverlapped);

	private:
		ByteQueueSend sendBuf;
		ByteQueueRecv recvBuf;
		Overlapped sendOverlapped;
		Overlapped recvOverlapped;
	};
}