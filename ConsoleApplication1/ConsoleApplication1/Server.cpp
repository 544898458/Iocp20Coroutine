#include <Winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <winnt.h>
#include "Server.h"
#include"SocketCompeletionKey.h"
#pragma comment(lib,"ws2_32.lib")

bool Iocp::Server::WsaStartup()
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
			printf("�����µ������ԣ����߼�������");
			break;
		case WSAVERNOTSUPPORTED:
			printf("����������");
			break;
		case WSAEINPROGRESS:
			printf("����������");
			break;
		case WSAEPROCLIM:
			printf("�볢�Թص�����Ҫ���������Ϊ��ǰ���������ṩ������Դ");
			break;
		}

		return false;
	}
}

bool Iocp::Server::Init()
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
		//���������
		//WSACleanup();
		return false;
	}
	//������ɶ˿�	����һ��I/O��ɶ˿ڶ����������������������׽��־����������I/O����Ҫ������һ�㣬��Ҫ����CreateCompletionPort������
	this->hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == this->hIocp)
	{
		int a = GetLastError();
		printf("%d\n", a);
		closesocket(socketAccept);
		//���������
		//WSACleanup();
		return false;
	}
	auto pCompleteKey = new ListenSocketCompeletionKey(this->socketAccept);
	//��
	const auto iocp = CreateIoCompletionPort((HANDLE)socketAccept, this->hIocp, (ULONG_PTR)pCompleteKey, 0);
	if (iocp != this->hIocp)
	{
		int a = GetLastError();
		printf("��ɶ˿ڰ�socketʧ��%d\n", a);

		CloseHandle(iocp);
		closesocket(socketAccept);
		//���������
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
		//������
		int a = WSAGetLastError();
		const char* szErr = "";
		switch (a) {
		case WSAEADDRNOTAVAIL:szErr = "The requested address is not valid in its context.�����������У�������ĵ�ַ��Ч��"; break;
		}
		printf("a=%d,%s", a, szErr);


		//�ͷ�
		closesocket(socketAccept);
		//���������
		WSACleanup();
		return false;
	}

	if (SOCKET_ERROR == listen(socketAccept, SOMAXCONN))
	{
		//������
		int a = WSAGetLastError();
		//�ͷ�
		CloseHandle(iocp);
		closesocket(socketAccept);
		//���������
		WSACleanup();
		return 0;
	}
	//�����߳���������
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
			//���������
			//WSACleanup();
			return false;
		}
		this->vecThread.push_back(hThread);
	}

	//if ( !
	pCompleteKey->StartCoRoutine();
		//)
	//{
	//	//Clear();
	//	//���������
	//	WSACleanup();
	//	return false;
	//}


	return true;
}

DWORD WINAPI Iocp::Server::ThreadProc(LPVOID lpParameter)
{
	auto* pThis = (Server*)lpParameter;
	HANDLE port = pThis->hIocp;
	DWORD      number_of_bytes = 0;
	SocketCompeletionKey* CompletionKey = nullptr;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&CompletionKey, &lpOverlapped, INFINITE);//û��ɾͻῨ���������
		//if (FALSE == bFlag)
		//{
		//	
		//	if (64 == a)
		//	{
		//		printf("force close\n");
		//	}
		//	printf("%d\n", a);
		//	continue;
		//}
		int lastErr = GetLastError();//������Socketǿ�ƹر�
		//����
		//accept
		//auto socket = CONTAINING_RECORD(lpOverlapped, MyOverlapped, socket);
		auto* overlapped = (MyOverlapped*)lpOverlapped;
		overlapped->OnComplete(CompletionKey,port,number_of_bytes, bFlag, lastErr);
	}

	return 0;
}

