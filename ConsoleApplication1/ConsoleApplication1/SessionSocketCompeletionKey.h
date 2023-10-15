#pragma once
#include "SocketCompeletionKey.h"
#include<WinSock2.h>
#include "ByteQueue.h"
template<class T_Session>
class SessionSocketCompeletionKey :public SocketCompeletionKey
{
public:
	SessionSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
	{

	}
	virtual void StartCoRoutine() override;
private:
	CoTask<int> PostSend(MyOverlapped* pOverlapped);
	CoTask<int> PostRecv(MyOverlapped* pOverlapped);
	std::tuple<bool, bool>  WSASend(MyOverlapped* pOverlapped);
	bool WSARecv(MyOverlapped* pOverlapped);
	
private:
	T_Session Session;
	ByteQueueSend sendBuf;
	ByteQueueRecv recvBuf;
	MyOverlapped* pSendOverlapped;
	MyOverlapped* pRecvOverlapped;
};