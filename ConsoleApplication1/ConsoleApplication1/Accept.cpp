#include <Winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <winnt.h>
#include "Accept.h"
#define MAX_RECV_COUNT  1024

bool Iocp::Accept::WsaStartup()
{
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdScokMsg;
	int nRes = WSAStartup(wdVersion, &wdScokMsg);

	if (0 == nRes)
		return true;
	{
		switch (nRes)
		{
		case WSASYSNOTREADY:
			printf("重启下电脑试试，或者检查网络库");
			break;
		case WSAVERNOTSUPPORTED:
			printf("请更新网络库");
			break;
		case WSAEINPROGRESS:
			printf("请重新启动");
			break;
		case WSAEPROCLIM:
			printf("请尝试关掉不必要的软件，以为当前网络运行提供充足资源");
			break;
		}

		return false;
	}
}

bool Iocp::Accept::Init()
{
	if (this->socketAccept == NULL)
		return false;

	this->socketAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int a = WSAGetLastError();
	if (INVALID_SOCKET == socketAccept)
	{
		int a = WSAGetLastError();
		//清理网络库
		//WSACleanup();
		return false;
	}

	struct sockaddr_in si = { };
	si.sin_family = AF_INET;
	si.sin_port = htons(12345);
	si.sin_addr.S_un.S_addr = inet_pton(si.sin_family, "127.0.0.1", (PVOID)si.sin_addr.S_un.S_addr);// inet_pton("127.0.0.1");
	//int a = ~0;
	if (SOCKET_ERROR == bind(socketAccept, (const struct sockaddr*)&si, sizeof(si)))
	{
		//出错了
		int a = WSAGetLastError();
		//释放
		closesocket(socketAccept);
		//清理网络库
		WSACleanup();
		return false;
	}


	//创建完成端口	创建一个I/O完成端口对象，用它面向任意数量的套接字句柄，管理多个I/O请求。要做到这一点，需要调用CreateCompletionPort函数。
	this->hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == this->hIocp)
	{
		int a = GetLastError();
		printf("%d\n", a);
		closesocket(socketAccept);
		//清理网络库
		//WSACleanup();
		return false;
	}
	//绑定
	const auto iocp = CreateIoCompletionPort((HANDLE)socketAccept, this->hIocp, this->socketAccept, 0);
	if (iocp != this->hIocp)
	{
		int a = GetLastError();
		printf("完成端口绑定socket失败%d\n", a);

		CloseHandle(iocp);
		closesocket(socketAccept);
		//清理网络库
		WSACleanup();
		return 0;
	}


	if (SOCKET_ERROR == listen(socketAccept, SOMAXCONN))
	{
		//出错了
		int a = WSAGetLastError();
		//释放
		CloseHandle(iocp);
		closesocket(socketAccept);
		//清理网络库
		WSACleanup();
		return 0;
	}

	if (0 != PostAccept())
	{
		//Clear();
		//清理网络库
		WSACleanup();
		return 0;
	}
	//创建线程数量有了
	SYSTEM_INFO system_processors_count;
	GetSystemInfo(&system_processors_count);
	auto process_count = system_processors_count.dwNumberOfProcessors;

	//创建
	//pThread = (HANDLE*)malloc(sizeof(HANDLE) * process_count);

	for (int i = 0; i < process_count; i++)
	{
		auto hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
		if (NULL == hThread)
		{
			int a = GetLastError();
			printf("%d\n", a);
			CloseHandle(iocp);
			closesocket(socketAccept);
			//清理网络库
			//WSACleanup();
			return false;
		}
		this->vecThread.push_back(hThread);
	}
}
bool Iocp::Accept::PostAccept()
{
	auto pSession = new MyOverlapped();
	pSession->op = MyOverlapped::Accept;
	pSession->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//all_olp[count].hEvent = WSACreateEvent();

	char str[1024] = { 0 };
	DWORD dwRecvcount;

	BOOL bRes = AcceptEx(
		this->socketAccept,	//[in]侦听套接字。服务器应用程序在这个套接字上等待连接。
		pSession->socket,	//[in]将用于连接的套接字。此套接字必须不能已经绑定或者已经连接。
		str, //[in]指向一个缓冲区，该缓冲区用于接收新建连接的所发送数据的第一个块、该服务器的本地地址和客户端的远程地址。接收到的数据将被写入到缓冲区0偏移处，而地址随后写入。 该参数必须指定，如果此参数设置为NULL，将不会得到执行，也无法通过GetAcceptExSockaddrs函数获得本地或远程的地址。
		0,	//[in]lpOutputBuffer字节数，指定接收数据缓冲区lpOutputBuffer的大小。这一大小应不包括服务器的本地地址的大小或客户端的远程地址，他们被追加到输出缓冲区。如果dwReceiveDataLength是零，AcceptEx将不等待接收任何数据，而是尽快建立连接。
		sizeof(struct sockaddr_in) + 16,//[in]为本地地址信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。
		sizeof(struct sockaddr_in) + 16,//[in]为远程地址的信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。 该值不能为0。
		&dwRecvcount,//[out]指向一个DWORD用于标识接收到的字节数。此参数只有在同步模式下有意义。如果函数返回ERROR_IO_PENDING并在迟些时候完成操作，那么这个DWORD没有意义，这时你必须获得从完成通知机制中读取操作字节数。
		&pSession->overlapped
	);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//函数执行出错
		printf("%d\n", a);
		return false;
	}
	return 0;
}

DWORD WINAPI Iocp::Accept::ThreadProc(LPVOID lpParameter)
{
	auto* pThis = (Accept*)lpParameter;
	HANDLE port = pThis->hIocp;
	DWORD      number_of_bytes;
	SOCKET socket;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, &socket, &lpOverlapped, INFINITE);
		if (FALSE == bFlag)
		{
			int a = GetLastError();
			if (64 == a)
			{
				printf("force close\n");
			}
			printf("%d\n", a);
			continue;
		}
		//处理
		//accept
		//auto socket = CONTAINING_RECORD(lpOverlapped, MyOverlapped, socket);
		auto* overlapped = (MyOverlapped*)lpOverlapped;
		if (overlapped->op== MyOverlapped::Accept)
		{
			printf("accept\n");
			//绑定到完成端口
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)overlapped->socket, port, overlapped->socket, 0);
			if (hPort1 != port)
			{
				int a = GetLastError();
				printf("%d\n", a);
				closesocket(overlapped->socket);// all_socks[count]);
				continue;
			}
			PostSend(count);
			//新客户端投递recv
			PostRecv(count);
			count++;
			pThis->PostAccept();
		}
		else
		{
			if (0 == number_of_bytes)
			{
				//客户端下线
				printf("close\n");
				//关闭
				closesocket(all_socks[socket]);
				WSACloseEvent(all_olp[socket].hEvent);
				//从数组中删掉
				all_socks[socket] = 0;
				all_olp[socket].hEvent = NULL;
			}
			else
			{
				if (0 != recv_buf[0])
				{
					//收到  recv
					printf("%s\n", recv_buf);
					memset(recv_buf, 0, sizeof(recv_buf));
					//
					PostRecv(socket);
				}
				else
				{
					//send
					printf("send ok\n");
				}
			}
		}
	}

	return 0;
}
bool Iocp::Accept::PostSend(int index)
{
	WSABUF wsabuf;
	wsabuf.buf = (CHAR*)"你好";
	wsabuf.len = MAX_RECV_COUNT;

	DWORD dwSendCount;
	DWORD dwFlag = 0;
	int nRes = WSASend(all_socks[index], &wsabuf, 1, &dwSendCount, dwFlag, &all_olp[index], NULL);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//延迟处理
		//函数执行出错
		return 1;
	}
	return 0;
}