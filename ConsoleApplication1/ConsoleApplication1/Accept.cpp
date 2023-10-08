#include <Winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
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
	if (this->socketServer == NULL)
		return false;

	this->socketServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int a = WSAGetLastError();
	if (INVALID_SOCKET == socketServer)
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
	if (SOCKET_ERROR == bind(socketServer, (const struct sockaddr*)&si, sizeof(si)))
	{
		//������
		int a = WSAGetLastError();
		//�ͷ�
		closesocket(socketServer);
		//���������
		WSACleanup();
		return false;
	}


	//������ɶ˿�
	this->hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == this->hIocp)
	{
		int a = GetLastError();
		printf("%d\n", a);
		closesocket(socketServer);
		//���������
		//WSACleanup();
		return false;
	}
	//��
	HANDLE hIocpAccept = CreateIoCompletionPort((HANDLE)socketServer, this->hIocp, 0, 0);
	if (hPort1 != this->hIocp)
	{
		int a = GetLastError();
		printf("%d\n", a);

		CloseHandle(hPort);
		closesocket(socketServer);
		//���������
		WSACleanup();
		return 0;
	}
}
