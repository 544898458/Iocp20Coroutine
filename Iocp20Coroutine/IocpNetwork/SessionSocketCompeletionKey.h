#pragma once
#include "SocketCompeletionKey.h"
#include<WinSock2.h>
#include <set>
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
		void StartCoRoutine();
		void Send(const void* buf, int len);
		bool Finished();
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
		std::mutex lockFinish;
		bool sendFinish = false;
		bool recvFinish = false;
	};
}
