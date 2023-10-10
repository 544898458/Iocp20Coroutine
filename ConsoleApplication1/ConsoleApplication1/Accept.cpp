#include <Winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <winnt.h>
#include "Accept.h"

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
	if (this->socketAccept == NULL)
		return false;

	this->socketAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int a = WSAGetLastError();
	if (INVALID_SOCKET == socketAccept)
	{
		int a = WSAGetLastError();
		//���������
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
		//������
		int a = WSAGetLastError();
		//�ͷ�
		closesocket(socketAccept);
		//���������
		WSACleanup();
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

	if (0 != PostAccept())
	{
		//Clear();
		//���������
		WSACleanup();
		return 0;
	}
	//�����߳���������
	SYSTEM_INFO system_processors_count;
	GetSystemInfo(&system_processors_count);
	auto process_count = system_processors_count.dwNumberOfProcessors;

	//����
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
			//���������
			//WSACleanup();
			return false;
		}
		this->vecThread.push_back(hThread);
	}
}
bool Iocp::Accept::PostAccept()
{
	auto pAcceptOverlapped = new MyOverlapped(MyOverlapped::Accept);
	
	pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//all_olp[count].hEvent = WSACreateEvent();

	char str[1024] = { 0 };
	DWORD dwRecvcount;

	BOOL bRes = AcceptEx(
		this->socketAccept,	//[in]�����׽��֡�������Ӧ�ó���������׽����ϵȴ����ӡ�
		pAcceptOverlapped->socket,	//[in]���������ӵ��׽��֡����׽��ֱ��벻���Ѿ��󶨻����Ѿ����ӡ�
		str, //[in]ָ��һ�����������û��������ڽ����½����ӵ����������ݵĵ�һ���顢�÷������ı��ص�ַ�Ϳͻ��˵�Զ�̵�ַ�����յ������ݽ���д�뵽������0ƫ�ƴ�������ַ���д�롣 �ò�������ָ��������˲�������ΪNULL��������õ�ִ�У�Ҳ�޷�ͨ��GetAcceptExSockaddrs������ñ��ػ�Զ�̵ĵ�ַ��
		0,	//[in]lpOutputBuffer�ֽ�����ָ���������ݻ�����lpOutputBuffer�Ĵ�С����һ��СӦ�������������ı��ص�ַ�Ĵ�С��ͻ��˵�Զ�̵�ַ�����Ǳ�׷�ӵ���������������dwReceiveDataLength���㣬AcceptEx�����ȴ������κ����ݣ����Ǿ��콨�����ӡ�
		sizeof(struct sockaddr_in) + 16,//[in]Ϊ���ص�ַ��Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ�
		sizeof(struct sockaddr_in) + 16,//[in]ΪԶ�̵�ַ����Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ� ��ֵ����Ϊ0��
		&dwRecvcount,//[out]ָ��һ��DWORD���ڱ�ʶ���յ����ֽ������˲���ֻ����ͬ��ģʽ�������塣�����������ERROR_IO_PENDING���ڳ�Щʱ����ɲ�������ô���DWORDû�����壬��ʱ������ô����֪ͨ�����ж�ȡ�����ֽ�����
		&pAcceptOverlapped->overlapped
	);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//����ִ�г���
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
	MyCompeletionKey *CompletionKey=nullptr;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)CompletionKey, &lpOverlapped, INFINITE);
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
		//����
		//accept
		//auto socket = CONTAINING_RECORD(lpOverlapped, MyOverlapped, socket);
		auto* overlapped = (MyOverlapped*)lpOverlapped;
		if (overlapped->op== MyOverlapped::Accept)
		{
			printf("accept\n");
			//�󶨵���ɶ˿�
			auto pCompleteKey = new MyCompeletionKey();
			pCompleteKey->socket = CompletionKey->socket;
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)CompletionKey->socket, port, (ULONG_PTR)pCompleteKey, 0);
			if (hPort1 != port)
			{
				int a = GetLastError();
				printf("%d\n", a);
				closesocket(CompletionKey->socket);// all_socks[count]);
				continue;
			}

			auto pOverlapped = new MyOverlapped(MyOverlapped::Send);
			pThis->PostSend(CompletionKey, pOverlapped);
			//�¿ͻ���Ͷ��recv
			pThis->PostRecv(CompletionKey);
			//count++;
			pThis->PostAccept();
		}
		else
		{
			if (0 == number_of_bytes)
			{
				//�ͻ�������
				printf("close\n");
				//�ر�
				closesocket(CompletionKey->socket);
				//WSACloseEvent(all_olp[CompletionKey].hEvent);
				//��������ɾ��
				//all_socks[CompletionKey] = 0;
				//all_olp[CompletionKey].hEvent = NULL;
			}
			else
			{
				if (0 != CompletionKey->recv_buf[0])
				{
					//�յ�  recv
					printf("%s\n", CompletionKey->recv_buf);
					memset(CompletionKey->recv_buf, 0, sizeof(CompletionKey->recv_buf));
					//
					pThis->PostRecv(CompletionKey);
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
bool Iocp::Accept::PostSend(MyCompeletionKey* pKey,MyOverlapped *pOverlapped)
{
	WSABUF wsabuf;
	wsabuf.buf = (CHAR*)"���";
	wsabuf.len = MAX_RECV_COUNT;

	DWORD dwSendCount;
	DWORD dwFlag = 0;
	int nRes = WSASend(pKey->socket, &wsabuf, 1, &dwSendCount, dwFlag, &pOverlapped->overlapped, NULL);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//�ӳٴ���
		//����ִ�г���
		return 1;
	}
	return 0;
}


bool Iocp::Accept::PostRecv(MyCompeletionKey *pKey)
{
	WSABUF wsabuf;
	wsabuf.buf = pKey->recv_buf;
	wsabuf.len = MAX_RECV_COUNT;

	DWORD dwRecvCount;
	DWORD dwFlag = 0;
	auto pOverlapped = new MyOverlapped(MyOverlapped::Recv);
	int nRes = WSARecv(pKey->socket, &wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//����ִ�г���
		return false;
	}
	return true;
}