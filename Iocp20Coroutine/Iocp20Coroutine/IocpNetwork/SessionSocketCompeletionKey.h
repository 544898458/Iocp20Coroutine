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
		virtual void StartCoRoutine() override;
		void Send(const char buf[], int len);
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
/// <summary>
/// 所有活动连接
/// </summary>
template<class T_Session>
std::set<Iocp::SessionSocketCompeletionKey<T_Session>*> g_setSession;
/// <summary>
/// 多线程全局操作g_setSession
/// </summary>
template<class T_Session>
std::mutex g_setSessionMutex;


/// <summary>
/// 向所有连接广播消息
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="msg"></param>
template<class T,class T_Session>
void Broadcast(const T& msg);