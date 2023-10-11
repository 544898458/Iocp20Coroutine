#pragma once
#include<WinSock2.h>
#include"Define.h"
#include"MyOverlapped.h"
/// <summary>
	/// 对应一个Socket，可能是监听Socket也可能是连接Socket
	/// </summary>
struct MyCompeletionKey {
	SOCKET socket;
	char recv_buf[MAX_RECV_COUNT];
	bool PostSend(MyCompeletionKey* pKey, MyOverlapped* pOverlapped)
	{
		WSABUF wsabuf;
		wsabuf.buf = (CHAR*)"你好";
		wsabuf.len = MAX_RECV_COUNT;

		DWORD dwSendCount;
		DWORD dwFlag = 0;
		int nRes = WSASend(pKey->socket, &wsabuf, 1, &dwSendCount, dwFlag, &pOverlapped->overlapped, NULL);

		int a = WSAGetLastError();
		if (a == 0)
		{
			//同步发送完成
			PostSend(pKey, pOverlapped);
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

};