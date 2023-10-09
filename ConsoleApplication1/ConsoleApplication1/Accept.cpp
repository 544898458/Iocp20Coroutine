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
	//��
	const auto iocp = CreateIoCompletionPort((HANDLE)socketAccept, this->hIocp, this->socketAccept, 0);
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
	auto pSession = new MyOverlapped();
	pSession->op = MyOverlapped::Accept;
	pSession->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//all_olp[count].hEvent = WSACreateEvent();

	char str[1024] = { 0 };
	DWORD dwRecvcount;

	BOOL bRes = AcceptEx(
		this->socketAccept,	//[in]�����׽��֡�������Ӧ�ó���������׽����ϵȴ����ӡ�
		pSession->socket,	//[in]���������ӵ��׽��֡����׽��ֱ��벻���Ѿ��󶨻����Ѿ����ӡ�
		str, //[in]ָ��һ�����������û��������ڽ����½����ӵ����������ݵĵ�һ���顢�÷������ı��ص�ַ�Ϳͻ��˵�Զ�̵�ַ�����յ������ݽ���д�뵽������0ƫ�ƴ�������ַ���д�롣 �ò�������ָ��������˲�������ΪNULL��������õ�ִ�У�Ҳ�޷�ͨ��GetAcceptExSockaddrs������ñ��ػ�Զ�̵ĵ�ַ��
		0,	//[in]lpOutputBuffer�ֽ�����ָ���������ݻ�����lpOutputBuffer�Ĵ�С����һ��СӦ�������������ı��ص�ַ�Ĵ�С��ͻ��˵�Զ�̵�ַ�����Ǳ�׷�ӵ���������������dwReceiveDataLength���㣬AcceptEx�����ȴ������κ����ݣ����Ǿ��콨�����ӡ�
		sizeof(struct sockaddr_in) + 16,//[in]Ϊ���ص�ַ��Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ�
		sizeof(struct sockaddr_in) + 16,//[in]ΪԶ�̵�ַ����Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ� ��ֵ����Ϊ0��
		&dwRecvcount,//[out]ָ��һ��DWORD���ڱ�ʶ���յ����ֽ������˲���ֻ����ͬ��ģʽ�������塣�����������ERROR_IO_PENDING���ڳ�Щʱ����ɲ�������ô���DWORDû�����壬��ʱ������ô����֪ͨ�����ж�ȡ�����ֽ�����
		&pSession->overlapped
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
		//����
		//accept
		//auto socket = CONTAINING_RECORD(lpOverlapped, MyOverlapped, socket);
		auto* overlapped = (MyOverlapped*)lpOverlapped;
		if (overlapped->op== MyOverlapped::Accept)
		{
			printf("accept\n");
			//�󶨵���ɶ˿�
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)overlapped->socket, port, overlapped->socket, 0);
			if (hPort1 != port)
			{
				int a = GetLastError();
				printf("%d\n", a);
				closesocket(overlapped->socket);// all_socks[count]);
				continue;
			}
			PostSend(count);
			//�¿ͻ���Ͷ��recv
			PostRecv(count);
			count++;
			pThis->PostAccept();
		}
		else
		{
			if (0 == number_of_bytes)
			{
				//�ͻ�������
				printf("close\n");
				//�ر�
				closesocket(all_socks[socket]);
				WSACloseEvent(all_olp[socket].hEvent);
				//��������ɾ��
				all_socks[socket] = 0;
				all_olp[socket].hEvent = NULL;
			}
			else
			{
				if (0 != recv_buf[0])
				{
					//�յ�  recv
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
	wsabuf.buf = (CHAR*)"���";
	wsabuf.len = MAX_RECV_COUNT;

	DWORD dwSendCount;
	DWORD dwFlag = 0;
	int nRes = WSASend(all_socks[index], &wsabuf, 1, &dwSendCount, dwFlag, &all_olp[index], NULL);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//�ӳٴ���
		//����ִ�г���
		return 1;
	}
	return 0;
}