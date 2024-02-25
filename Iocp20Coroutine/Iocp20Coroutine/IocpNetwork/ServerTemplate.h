#include "Server.h"
#include "ListenSocketCompeletionKey.h"
/// <summary>
/// 
/// </summary>
/// <typeparam name="T_Session"></typeparam>
/// <returns></returns>
template<class T_Session>
	requires requires(T_Session& refSession)
{
	std::is_function_v<decltype(T_Session::OnInit)>;
	std::is_function_v<decltype(T_Session::OnRecv)>;
	std::is_function_v<decltype(T_Session::OnDestroy)>;
}
bool Iocp::Server::Init()
{
	if (this->m_socketAccept != NULL)
		return false;

	this->m_socketAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int a = WSAGetLastError();
	if (INVALID_SOCKET == m_socketAccept)
	{
		int a = WSAGetLastError();
		//���������
		//WSACleanup();
		return false;
	}

	//auto pListenCompleteKey = new ListenSocketCompeletionKey<T_Session>(this->socketAccept);
	//������ɶ˿�	����һ��I/O��ɶ˿ڶ����������������������׽��־����������I/O����Ҫ������һ�㣬��Ҫ����CreateCompletionPort������
	this->m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == m_hIocp)
	{
		int a = GetLastError();
		LOG(INFO) << a;
		closesocket(m_socketAccept);
		//���������
		//WSACleanup();
		return false;
	}
	//��
	const auto iocp = CreateIoCompletionPort((HANDLE)m_socketAccept, m_hIocp, (ULONG_PTR)0, 0);
	if (iocp != m_hIocp)
	{
		int a = GetLastError();
		LOG(INFO) << "��ɶ˿ڰ�socketʧ��" << a;

		CloseHandle(iocp);
		closesocket(m_socketAccept);
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
	if (SOCKET_ERROR == ::bind(m_socketAccept, (sockaddr*)&si, sizeof(si)))
	{
		//������
		int a = WSAGetLastError();
		const char* szErr = "";
		switch (a) {
		case WSAEADDRNOTAVAIL:szErr = "The requested address is not valid in its context.�����������У�������ĵ�ַ��Ч��"; break;
		}
		LOG(INFO) << "a=" << a << "," << szErr;


		//�ͷ�
		closesocket(m_socketAccept);
		//���������
		WSACleanup();
		return false;
	}

	if (SOCKET_ERROR == listen(m_socketAccept, SOMAXCONN))
	{
		//������
		int a = WSAGetLastError();
		//�ͷ�
		CloseHandle(iocp);
		closesocket(m_socketAccept);
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
		std::thread networkThread(Server::NetworkThreadProc, m_hIocp);
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
	ListenSocketCompeletionKey::StartCoRoutine<T_Session>(m_hIocp, m_socketAccept);
	//)
//{
//	//Clear();
//	//���������
//	WSACleanup();
//	return false;
//}


	return true;
}