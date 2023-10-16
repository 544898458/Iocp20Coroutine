#include <Winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <winnt.h>
#include "Server.h"
#include "ListenSocketCompeletionKey.h"
#pragma comment(lib,"ws2_32.lib")

template<class T_Session>
bool Iocp::Server<T_Session>::WsaStartup()
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
template<class T_Session>
bool Iocp::Server<T_Session>::Init()
{
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
	
	auto pListenCompleteKey = new ListenSocketCompeletionKey<T_Session>(this->socketAccept);
	//创建完成端口	创建一个I/O完成端口对象，用它面向任意数量的套接字句柄，管理多个I/O请求。要做到这一点，需要调用CreateCompletionPort函数。
	pListenCompleteKey->hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == pListenCompleteKey->hIocp)
	{
		int a = GetLastError();
		printf("%d\n", a);
		closesocket(socketAccept);
		//清理网络库
		//WSACleanup();
		return false;
	}
	//绑定
	const auto iocp = CreateIoCompletionPort((HANDLE)socketAccept, pListenCompleteKey->hIocp, (ULONG_PTR)pListenCompleteKey, 0);
	if (iocp != pListenCompleteKey->hIocp)
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
		//auto hThread = CreateThread(NULL, 0, NetworkThreadProc, pListenCompleteKey->hIocp, 0, NULL);
		thread networkThread(Server::NetworkThreadProc, this, pListenCompleteKey->hIocp);
		networkThread.detach();
		//if (NULL == hThread)
		//{
		//	int a = GetLastError();
		//	printf("%d\n", a);
		//	CloseHandle(iocp);
		//	closesocket(socketAccept);
		//	//清理网络库
		//	//WSACleanup();
		//	return false;
		//}
		//this->vecThread.push_back(hThread);
	}

	//if ( !
	pListenCompleteKey->StartCoRoutine();
		//)
	//{
	//	//Clear();
	//	//清理网络库
	//	WSACleanup();
	//	return false;
	//}


	return true;
}

template<class T_Session>
void Iocp::Server<T_Session>::NetworkThreadProc(HANDLE port)
{
	//auto hIocp= (HANDLE)lpParameter;
	//HANDLE port = hIocp;
	DWORD      number_of_bytes = 0;
	SocketCompeletionKey* CompletionKey = nullptr;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&CompletionKey, &lpOverlapped, INFINITE);//没完成就会卡在这里，正常
		int lastErr = GetLastError();//可能是Socket强制关闭
		auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
		overlapped->OnComplete(CompletionKey,port,number_of_bytes, bFlag, lastErr);
	}

	return ;
}

