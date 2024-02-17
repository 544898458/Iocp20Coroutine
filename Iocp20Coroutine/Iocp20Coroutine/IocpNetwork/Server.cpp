#include <glog/logging.h>

#include <Winsock2.h>

#include <WS2tcpip.h>
#include <mswsock.h>
#include <winnt.h>
#include "Server.h"
#include "./ListenSocketCompeletionKey.h"
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
			LOG(INFO) << "�����µ������ԣ����߼�������";
			break;
		case WSAVERNOTSUPPORTED:
			LOG(INFO) << ("����������");
			break;
		case WSAEINPROGRESS:
			LOG(INFO) << ("����������");
			break;
		case WSAEPROCLIM:
			LOG(INFO) << ("�볢�Թص�����Ҫ���������Ϊ��ǰ���������ṩ������Դ");
			break;
		}

		return false;
	}
}

/// <summary>
/// 
/// </summary>
/// <typeparam name="T_Session"></typeparam>
/// <returns></returns>
template<class T_Session>
bool Iocp::Server<T_Session>::Init()
{
	if (this->socketAccept != NULL)
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
	
	auto pListenCompleteKey = new ListenSocketCompeletionKey<T_Session>(this->socketAccept);
	//������ɶ˿�	����һ��I/O��ɶ˿ڶ����������������������׽��־����������I/O����Ҫ������һ�㣬��Ҫ����CreateCompletionPort������
	pListenCompleteKey->hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == pListenCompleteKey->hIocp)
	{
		int a = GetLastError();
		LOG(INFO) <<  a;
		closesocket(socketAccept);
		//���������
		//WSACleanup();
		return false;
	}
	//��
	const auto iocp = CreateIoCompletionPort((HANDLE)socketAccept, pListenCompleteKey->hIocp, (ULONG_PTR)pListenCompleteKey, 0);
	if (iocp != pListenCompleteKey->hIocp)
	{
		int a = GetLastError();
		LOG(INFO) << "��ɶ˿ڰ�socketʧ��" << a;

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
	//inet_pton(AF_INET, "127.0.0.1", &si.sin_addr.S_un.S_addr);// inet_pton("127.0.0.1");
	si.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//int a = ~0;
	if (SOCKET_ERROR == ::bind(socketAccept, (sockaddr*)&si, sizeof(si)))
	{
		//������
		int a = WSAGetLastError();
		const char* szErr = "";
		switch (a) {
		case WSAEADDRNOTAVAIL:szErr = "The requested address is not valid in its context.�����������У�������ĵ�ַ��Ч��"; break;
		}
		LOG(INFO) << "a=" << a <<","<< szErr;


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
		//auto hThread = CreateThread(NULL, 0, NetworkThreadProc, pListenCompleteKey->hIocp, 0, NULL);
		thread networkThread(Server::NetworkThreadProc, pListenCompleteKey->hIocp);
		networkThread.detach();
		//if (NULL == hThread)
		//{
		//	int a = GetLastError();
		//	printf("%d\n", a);
		//	CloseHandle(iocp);
		//	closesocket(socketAccept);
		//	//���������
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
	//	//���������
	//	WSACleanup();
	//	return false;
	//}


	return true;
}

template<class T_Session>
void Iocp::Server<T_Session>::Stop()
{
	closesocket(this->socketAccept);
	this->socketAccept = NULL;
}

template<class T_Session>
void Iocp::Server<T_Session>::NetworkThreadProc(HANDLE port)
{
	//auto hIocp= (HANDLE)lpParameter;
	//HANDLE port = hIocp;
	DWORD      number_of_bytes = 0;
	SocketCompeletionKey* pCompletionKey = nullptr;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		//pCompletionKey��Ӧһ��socket��lpOverlapped��Ӧһ���¼�
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&pCompletionKey, &lpOverlapped, INFINITE);//û��ɾͻῨ���������
		int lastErr = GetLastError();//������Socketǿ�ƹر�
		auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
		overlapped->OnComplete(pCompletionKey,port,number_of_bytes, bFlag, lastErr);
		if (!bFlag)
		{
			switch (lastErr)
			{
			case ERROR_OPERATION_ABORTED:
				LOG(WARNING) << "The I/O operation has been aborted because of either a thread exit or an application request.";
				break;
			}
			return;
		}
	}
}

