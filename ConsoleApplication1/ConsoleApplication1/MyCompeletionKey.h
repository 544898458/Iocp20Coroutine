#pragma once
#include<WinSock2.h>
#include <mswsock.h>

#include"Define.h"
#include"MyOverlapped.h"
#include<stdio.h>
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

	bool PostRecv(MyCompeletionKey* pKey, MyOverlapped* pOverlapped)
	{
		WSABUF wsabuf;
		wsabuf.buf = pKey->recv_buf;
		wsabuf.len = MAX_RECV_COUNT;

		DWORD dwRecvCount;
		DWORD dwFlag = 0;

		int nRes = WSARecv(pKey->socket, &wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);

		int a = WSAGetLastError();
		if (ERROR_IO_PENDING == a)
		{
			return true;
		}

		if (a == 0)//同步操作成功
		{
			PostRecv(pKey, pOverlapped);
			return true;
		}
		printf("PostRecv err");
		return false;
	}
	bool PostAccept(MyOverlapped * pAcceptOverlapped)
	{
		//auto pAcceptOverlapped = new MyOverlapped(new OpAccept());

		pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		//all_olp[count].hEvent = WSACreateEvent();

		char* str2 = new char[1024];// = { 0 };
		DWORD dwRecvcount = 0;
		int b = WSAGetLastError();
		//OVERLAPPED* Overlapped = new OVERLAPPED();
		BOOL bRes = AcceptEx(
			this->socket,	//[in]侦听套接字。服务器应用程序在这个套接字上等待连接。
			pAcceptOverlapped->socket,	//[in]将用于连接的套接字。此套接字必须不能已经绑定或者已经连接。
			str2, //[in]指向一个缓冲区，该缓冲区用于接收新建连接的所发送数据的第一个块、该服务器的本地地址和客户端的远程地址。接收到的数据将被写入到缓冲区0偏移处，而地址随后写入。 该参数必须指定，如果此参数设置为NULL，将不会得到执行，也无法通过GetAcceptExSockaddrs函数获得本地或远程的地址。
			0,	//[in]lpOutputBuffer字节数，指定接收数据缓冲区lpOutputBuffer的大小。这一大小应不包括服务器的本地地址的大小或客户端的远程地址，他们被追加到输出缓冲区。如果dwReceiveDataLength是零，AcceptEx将不等待接收任何数据，而是尽快建立连接。
			sizeof(struct sockaddr_in) + 16,//[in]为本地地址信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。
			sizeof(struct sockaddr_in) + 16,//[in]为远程地址的信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。 该值不能为0。
			&dwRecvcount,//[out]指向一个DWORD用于标识接收到的字节数。此参数只有在同步模式下有意义。如果函数返回ERROR_IO_PENDING并在迟些时候完成操作，那么这个DWORD没有意义，这时你必须获得从完成通知机制中读取操作字节数。
			&pAcceptOverlapped->overlapped//overlapped这里面内存必须清零，否则WSAGetLastError会返回ERROR_INVALID_HANDLE
		);
		const auto lastErr = WSAGetLastError();
		if (!bRes && lastErr!= ERROR_IO_PENDING)
		{
			switch (lastErr) 
			{
			default:
				printf("AcceptEx err=%d",lastErr);
				break;
			}
			return false;
		}


		return true;
	}
};