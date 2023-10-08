#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#include <Winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#define MAX_COUNT  1024
#define MAX_RECV_COUNT  1024

SOCKET all_socks[MAX_COUNT];
OVERLAPPED all_olp[MAX_COUNT];
int count;
HANDLE hPort;
HANDLE* pThread;
int process_count;

//接收缓冲区
char recv_buf[MAX_RECV_COUNT];

int PostAccept(void);
int PostRecv(int index);
int PostSend(int index);

void Clear(void)
{
	for (int i = 0; i < count; i++)
	{
		if (0 == all_socks[i])
			continue;
		closesocket(all_socks[i]);
		WSACloseEvent(all_olp[i].hEvent);
	}
}

BOOL g_flag = TRUE;

BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		//释放所有socket
		CloseHandle(hPort);
		Clear();

		g_flag = FALSE;

		//释放线程句柄
		for (int i = 0; i < process_count; i++)
		{
			//TerminateThread(pThread[i],);
			CloseHandle(pThread[i]);
		}
		free(pThread);

		break;
	}
	return TRUE;
}
DWORD WINAPI ThreadProc(LPVOID lpParameter);
#include "Accept.h"
int main(void)
{
	auto accept =new Iocp::Accept();
	accept.Init();
	SetConsoleCtrlHandler(fun, TRUE);
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdScokMsg;
	int nRes = WSAStartup(wdVersion, &wdScokMsg);

	if (0 != nRes)
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
		return  0;
	}

	//校验版本
	if (2 != HIBYTE(wdScokMsg.wVersion) || 2 != LOBYTE(wdScokMsg.wVersion))
	{
		//说明版本不对
		//清理网络库
		WSACleanup();
		return 0;
	}

	SOCKET socketServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int a = WSAGetLastError();
	if (INVALID_SOCKET == socketServer)
	{
		int a = WSAGetLastError();
		//清理网络库
		WSACleanup();
		return 0;
	}

	struct sockaddr_in si = { };
	si.sin_family = AF_INET;
	si.sin_port = htons(12345);
	si.sin_addr.S_un.S_addr = inet_pton(si.sin_family, "127.0.0.1",(PVOID)si.sin_addr.S_un.S_addr);// inet_pton("127.0.0.1");
	//int a = ~0;
	if (SOCKET_ERROR == bind(socketServer, (const struct sockaddr*)&si, sizeof(si)))
	{
		//出错了
		int a = WSAGetLastError();
		//释放
		closesocket(socketServer);
		//清理网络库
		WSACleanup();
		return 0;
	}

	all_socks[count] = socketServer;
	all_olp[count].hEvent = WSACreateEvent();
	count++;

	//创建完成端口
	hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == hPort)
	{
		int a = GetLastError();
		printf("%d\n", a);
		closesocket(socketServer);
		//清理网络库
		WSACleanup();
		return 0;
	}
	//绑定
	HANDLE hPort1 = CreateIoCompletionPort((HANDLE)socketServer, hPort, 0, 0);
	if (hPort1 != hPort)
	{
		int a = GetLastError();
		printf("%d\n", a);

		CloseHandle(hPort);
		closesocket(socketServer);
		//清理网络库
		WSACleanup();
		return 0;
	}

	if (SOCKET_ERROR == listen(socketServer, SOMAXCONN))
	{
		//出错了
		int a = WSAGetLastError();
		//释放
		CloseHandle(hPort);
		closesocket(socketServer);
		//清理网络库
		WSACleanup();
		return 0;
	}

	if (0 != PostAccept())
	{
		Clear();
		//清理网络库
		WSACleanup();
		return 0;
	}
	//创建线程数量有了
	SYSTEM_INFO system_processors_count;
	GetSystemInfo(&system_processors_count);
	process_count = system_processors_count.dwNumberOfProcessors;

	//创建
	pThread = (HANDLE*)malloc(sizeof(HANDLE) * process_count);

	for (int i = 0; i < process_count; i++)
	{
		pThread[i] = CreateThread(NULL, 0, ThreadProc, hPort, 0, NULL);
		if (NULL == pThread[i])
		{
			int a = GetLastError();
			printf("%d\n", a);
			CloseHandle(hPort);
			closesocket(socketServer);
			//清理网络库
			WSACleanup();
			return 0;
		}
	}

	//阻塞
	while (1)
	{
		Sleep(1000);
	}

	//释放线程句柄
	for (int i = 0; i < process_count; i++)
	{
		CloseHandle(pThread[i]);
	}
	free(pThread);


	CloseHandle(hPort);
	Clear();
	//清理网络库
	WSACleanup();

	system("pause");
	return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	HANDLE port = (HANDLE)lpParameter;
	DWORD      number_of_bytes;
	ULONG_PTR index;
	LPOVERLAPPED lpOverlapped;
	while (g_flag)
	{
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, &index, &lpOverlapped, INFINITE);
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
		if (0 == index)
		{
			printf("accept\n");
			//绑定到完成端口
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)all_socks[count], hPort, count, 0);
			if (hPort1 != hPort)
			{
				int a = GetLastError();
				printf("%d\n", a);
				closesocket(all_socks[count]);
				continue;
			}
			PostSend(count);
			//新客户端投递recv
			PostRecv(count);
			count++;
			PostAccept();
		}
		else
		{
			if (0 == number_of_bytes)
			{
				//客户端下线
				printf("close\n");
				//关闭
				closesocket(all_socks[index]);
				WSACloseEvent(all_olp[index].hEvent);
				//从数组中删掉
				all_socks[index] = 0;
				all_olp[index].hEvent = NULL;
			}
			else
			{
				if (0 != recv_buf[0])
				{
					//收到  recv
					printf("%s\n", recv_buf);
					memset(recv_buf, 0, sizeof(recv_buf));
					//
					PostRecv(index);
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

int PostAccept()
{
	all_socks[count] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	all_olp[count].hEvent = WSACreateEvent();

	char str[1024] = { 0 };
	DWORD dwRecvcount;

	BOOL bRes = AcceptEx(all_socks[0], all_socks[count], str, 0, sizeof(struct sockaddr_in) + 16,
		sizeof(struct sockaddr_in) + 16, &dwRecvcount, &all_olp[0]);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//函数执行出错
		return 1;
	}
	return 0;
}

int PostRecv(int index)
{
	WSABUF wsabuf;
	wsabuf.buf = recv_buf;
	wsabuf.len = MAX_RECV_COUNT;

	DWORD dwRecvCount;
	DWORD dwFlag = 0;
	int nRes = WSARecv(all_socks[index], &wsabuf, 1, &dwRecvCount, &dwFlag, &all_olp[index], NULL);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//函数执行出错
		return 1;
	}
	return 0;
}

int PostSend(int index)
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


