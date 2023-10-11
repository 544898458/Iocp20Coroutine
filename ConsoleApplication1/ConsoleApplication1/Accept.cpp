#include <Winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <winnt.h>
#include "Accept.h"
#include"MyCompeletionKey.h"
#pragma comment(lib,"ws2_32.lib")

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
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	DWORD dwBytes = 0;

	if (false) 
	{
		SOCKET    m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		//SYSTEM_INFO sysInfo;
		//GetSystemInfo(&sysInfo);
		//int g_ThreadCount = sysInfo.dwNumberOfProcessors * 2;
		auto g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);// g_ThreadCount);
		CreateIoCompletionPort((HANDLE)m_socket, g_hIOCP, (u_long)0, 0);
		sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(12345);
		inet_pton(server.sin_family, "127.0.0.1", &server.sin_addr.S_un.S_addr);// inet_pton("127.0.0.1");
		bind(m_socket, (sockaddr*)&server, sizeof(server));
		listen(m_socket, SOMAXCONN);
		
		auto client = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED); //socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (client == INVALID_SOCKET) {

			wprintf(L"Create accept socket failed with error: %u\n", WSAGetLastError());
			getchar();
			closesocket(m_socket);
			WSACleanup();
			return 1;
		}

		OVERLAPPED* Overlapped = new OVERLAPPED();
		char* str = new char[1024];// = { 0 };
		bool bRetVal = AcceptEx(m_socket, client, str,//per_io_data->wsabuf.buf,
			0,
			sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
			&dwBytes, Overlapped);
		const auto err = WSAGetLastError();
	}

	if (this->socketAccept != NULL)
		return false;

	//WSADATA wsaData;
	//WSAStartup(MAKEWORD(2, 2), &wsaData);
	this->socketAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int a = WSAGetLastError();
	if (INVALID_SOCKET == socketAccept)
	{
		int a = WSAGetLastError();
		//清理网络库
		//WSACleanup();
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
	auto pCompleteKey = new MyCompeletionKey();
	pCompleteKey->socket = this->socketAccept;
	//绑定
	const auto iocp = CreateIoCompletionPort((HANDLE)socketAccept, this->hIocp, (ULONG_PTR)pCompleteKey, 0);
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

	sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(12345);
	//inet_pton(AF_INET, param._pip4_dst, &param_init._address_dest.sin_addr.S_un.S_addr);
	inet_pton(AF_INET, "127.0.0.1", &si.sin_addr.S_un.S_addr);// inet_pton("127.0.0.1");
	//int a = ~0;
	if (SOCKET_ERROR == bind(socketAccept, (sockaddr*)&si, sizeof(si)))
	{
		//出错了
		int a = WSAGetLastError();
		const char* szErr = "";
		switch (a) {
		case WSAEADDRNOTAVAIL:szErr = "The requested address is not valid in its context.在其上下文中，该请求的地址无效。"; break;
		}
		printf("a=%d,%s", a, szErr);


		//释放
		closesocket(socketAccept);
		//清理网络库
		WSACleanup();
		return false;
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
	//创建线程数量有了
	SYSTEM_INFO system_processors_count;
	GetSystemInfo(&system_processors_count);
	auto process_count = system_processors_count.dwNumberOfProcessors;
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
	if (0 != PostAccept())
	{
		//Clear();
		//清理网络库
		WSACleanup();
		return 0;
	}


	//创建
	//pThread = (HANDLE*)malloc(sizeof(HANDLE) * process_count);


}
bool Iocp::Accept::PostAccept()
{
	//auto client = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED); //socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//if (client == INVALID_SOCKET) {

	//	wprintf(L"Create accept socket failed with error: %u\n", WSAGetLastError());
	//	getchar();
	//	closesocket(this->socketAccept);
	//	WSACleanup();
	//	return 1;
	//}

	//OVERLAPPED* Overlapped = new OVERLAPPED();
	//char* str = new char[1024];// = { 0 };
	//DWORD dwBytes(0);
	//bool bRetVal = AcceptEx(socketAccept, client, str,//per_io_data->wsabuf.buf,
	//	0,
	//	sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
	//	&dwBytes, Overlapped);
	//const auto err2 = WSAGetLastError();


	auto pAcceptOverlapped = new MyOverlapped(new OpAccept());

	pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//all_olp[count].hEvent = WSACreateEvent();

	char* str2 = new char[1024];// = { 0 };
	DWORD dwRecvcount = 0;
	int b = WSAGetLastError();
	OVERLAPPED* Overlapped = new OVERLAPPED();
	BOOL bRes = AcceptEx(
		this->socketAccept,	//[in]侦听套接字。服务器应用程序在这个套接字上等待连接。
		pAcceptOverlapped->socket,	//[in]将用于连接的套接字。此套接字必须不能已经绑定或者已经连接。
		str2, //[in]指向一个缓冲区，该缓冲区用于接收新建连接的所发送数据的第一个块、该服务器的本地地址和客户端的远程地址。接收到的数据将被写入到缓冲区0偏移处，而地址随后写入。 该参数必须指定，如果此参数设置为NULL，将不会得到执行，也无法通过GetAcceptExSockaddrs函数获得本地或远程的地址。
		0,	//[in]lpOutputBuffer字节数，指定接收数据缓冲区lpOutputBuffer的大小。这一大小应不包括服务器的本地地址的大小或客户端的远程地址，他们被追加到输出缓冲区。如果dwReceiveDataLength是零，AcceptEx将不等待接收任何数据，而是尽快建立连接。
		sizeof(struct sockaddr_in) + 16,//[in]为本地地址信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。
		sizeof(struct sockaddr_in) + 16,//[in]为远程地址的信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。 该值不能为0。
		&dwRecvcount,//[out]指向一个DWORD用于标识接收到的字节数。此参数只有在同步模式下有意义。如果函数返回ERROR_IO_PENDING并在迟些时候完成操作，那么这个DWORD没有意义，这时你必须获得从完成通知机制中读取操作字节数。
		&pAcceptOverlapped->overlapped//overlapped这里面内存必须清零，否则WSAGetLastError会返回ERROR_INVALID_HANDLE
	);

	int a = WSAGetLastError();
	switch (a) {
	case ERROR_INVALID_HANDLE:break;
	}
	if (ERROR_IO_PENDING != a)
	{
		//函数执行出错
		printf("AcceptEx error=%d\n", a);
		return false;
	}
	return 0;
}

DWORD WINAPI Iocp::Accept::ThreadProc(LPVOID lpParameter)
{
	auto* pThis = (Accept*)lpParameter;
	HANDLE port = pThis->hIocp;
	DWORD      number_of_bytes = 0;
	MyCompeletionKey* CompletionKey = nullptr;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&CompletionKey, &lpOverlapped, INFINITE);//没完成就会卡在这里，正常
		if (FALSE == bFlag)
		{
			int a = GetLastError();//可能是Socket强制关闭
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
		overlapped->OnComplete(CompletionKey,port);
	}

	return 0;
}

