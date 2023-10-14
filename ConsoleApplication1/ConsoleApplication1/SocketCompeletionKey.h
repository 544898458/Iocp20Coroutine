#pragma once
#include<WinSock2.h>
#include <mswsock.h>

#include"Define.h"
#include"MyOverlapped.h"
#include<stdio.h>
#include<assert.h>
#include"CoTask.h"
/// <summary>
	/// 对应一个Socket，可能是监听Socket也可能是连接Socket
	/// 监听Socket同时只有一个重叠操作Accept
	/// 连接Socket同时有两个重叠操作Recv和Accept
	/// </summary>
class SocketCompeletionKey 
{
public:
	SocketCompeletionKey(SOCKET s) :socket(s) {

	}
	virtual void StartCoRoutine() = 0;
protected:
	SOCKET Socket() const { return socket; }
private:
	virtual bool PostSend( MyOverlapped* pOverlapped) = 0;
	virtual bool PostRecv(MyOverlapped* pOverlapped) = 0;
	//virtual void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped) = 0;
public:
	HANDLE hIocp = nullptr;
private:
	SOCKET socket;
	
};


class SessionSocketCompeletionKey :public SocketCompeletionKey
{
public:
	SessionSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
	{

	}
	//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
	//{
	//	assert(!"SessionSocketCompeletionKey不能PostAccept");
	//	return false;
	//}
	virtual void StartCoRoutine() override
	{
		{
			auto pOverlapped = new MyOverlapped();
			PostSend(pOverlapped);
		}
		{
			auto pOverlapped = new MyOverlapped();
			//新客户端投递recv
			PostRecv(pOverlapped);
		}
		return ;
	}
	virtual bool PostSend(MyOverlapped* pOverlapped)override
	{
		WSABUF wsabuf;
		wsabuf.buf = (CHAR*)"你好";
		wsabuf.len = MAX_RECV_COUNT;

		DWORD dwSendCount;
		DWORD dwFlag = 0;
		int nRes = WSASend(Socket(), &wsabuf, 1, &dwSendCount, dwFlag, &pOverlapped->overlapped, NULL);

		int a = WSAGetLastError();
		if (a == 0)
		{
			//同步发送完成
			PostSend(pOverlapped);
			return true;
		}
		if (ERROR_IO_PENDING != a)
		{
			//延迟处理
			//函数执行出错
			switch (a) {
			case WSAENOTCONN:
				printf("A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied.\n");
				break;
			}
			return false;
		}
		return true;
	}
	virtual bool PostRecv( MyOverlapped* pOverlapped)override
	{
		WSABUF wsabuf;
		wsabuf.buf = recv_buf;
		wsabuf.len = MAX_RECV_COUNT;

		DWORD dwRecvCount;
		DWORD dwFlag = 0;

		int nRes = WSARecv(Socket(), &wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);

		int a = WSAGetLastError();
		if (ERROR_IO_PENDING == a)
		{
			return true;
		}

		if (a == 0)//同步操作成功
		{
			PostRecv(pOverlapped);
			return true;
		}
		printf("PostRecv err");
		return false;
	}
	//void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped)
	//{
	//	if (0 == number_of_bytes)
	//	{
	//		//客户端下线
	//		printf("客户端下线 close\n");
	//		//关闭
	//		closesocket(Socket());
	//		//WSACloseEvent(all_olp[CompletionKey].hEvent);
	//		//从数组中删掉
	//		//all_socks[CompletionKey] = 0;
	//		//all_olp[CompletionKey].hEvent = NULL;
	//	}
	//	else
	//	{
	//		//if (0 != recv_buf[0])
	//		{
	//			//收到  recv
	//			printf("重叠接受完成：Recv,%s\n", recv_buf);
	//			memset(recv_buf, 0, sizeof(recv_buf));
	//			//
	//			PostRecv(pOverlapped);
	//		}
	//		//else
	//		//{
	//		//	//send
	//		//	printf("send ok\n");
	//		//}
	//	}
	//}
private:
	char recv_buf[MAX_RECV_COUNT];

};