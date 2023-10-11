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
	auto pCompleteKey = new MyCompeletionKey();
	pCompleteKey->socket = this->socketAccept;
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
	if (0 != PostAccept())
	{
		//Clear();
		//���������
		WSACleanup();
		return 0;
	}


	//����
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
		this->socketAccept,	//[in]�����׽��֡�������Ӧ�ó���������׽����ϵȴ����ӡ�
		pAcceptOverlapped->socket,	//[in]���������ӵ��׽��֡����׽��ֱ��벻���Ѿ��󶨻����Ѿ����ӡ�
		str2, //[in]ָ��һ�����������û��������ڽ����½����ӵ����������ݵĵ�һ���顢�÷������ı��ص�ַ�Ϳͻ��˵�Զ�̵�ַ�����յ������ݽ���д�뵽������0ƫ�ƴ�������ַ���д�롣 �ò�������ָ��������˲�������ΪNULL��������õ�ִ�У�Ҳ�޷�ͨ��GetAcceptExSockaddrs������ñ��ػ�Զ�̵ĵ�ַ��
		0,	//[in]lpOutputBuffer�ֽ�����ָ���������ݻ�����lpOutputBuffer�Ĵ�С����һ��СӦ�������������ı��ص�ַ�Ĵ�С��ͻ��˵�Զ�̵�ַ�����Ǳ�׷�ӵ���������������dwReceiveDataLength���㣬AcceptEx�����ȴ������κ����ݣ����Ǿ��콨�����ӡ�
		sizeof(struct sockaddr_in) + 16,//[in]Ϊ���ص�ַ��Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ�
		sizeof(struct sockaddr_in) + 16,//[in]ΪԶ�̵�ַ����Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ� ��ֵ����Ϊ0��
		&dwRecvcount,//[out]ָ��һ��DWORD���ڱ�ʶ���յ����ֽ������˲���ֻ����ͬ��ģʽ�������塣�����������ERROR_IO_PENDING���ڳ�Щʱ����ɲ�������ô���DWORDû�����壬��ʱ������ô����֪ͨ�����ж�ȡ�����ֽ�����
		&pAcceptOverlapped->overlapped//overlapped�������ڴ�������㣬����WSAGetLastError�᷵��ERROR_INVALID_HANDLE
	);

	int a = WSAGetLastError();
	switch (a) {
	case ERROR_INVALID_HANDLE:break;
	}
	if (ERROR_IO_PENDING != a)
	{
		//����ִ�г���
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
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&CompletionKey, &lpOverlapped, INFINITE);//û��ɾͻῨ���������
		if (FALSE == bFlag)
		{
			int a = GetLastError();//������Socketǿ�ƹر�
			if (64 == a)
			{
				printf("force close\n");
			}
			printf("%d\n", a);
			continue;
		}
		//����
		//accept
		//auto socket = CONTAINING_RECORD(lpOverlapped, MyOverlapped, socket);
		auto* overlapped = (MyOverlapped*)lpOverlapped;
		overlapped->OnComplete(CompletionKey,port);
	}

	return 0;
}

