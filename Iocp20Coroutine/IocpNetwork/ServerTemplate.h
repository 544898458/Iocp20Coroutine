#include "Server.h"
#include "ListenSocketCompeletionKey.h"
#include "SessionSocketCompeletionKey.h"
#include "Client.h"
/// <summary>
/// 
/// </summary>
/// <typeparam name="T_Session"></typeparam>
/// <returns></returns>
template<class T_Server>
template<class T_Session>
	requires requires(Iocp::SessionSocketCompeletionKey<T_Session>& refCompletetionKeySession, T_Session& refSession, T_Server& refServer)
	{
		requires std::is_same_v<int, decltype(refSession.OnRecv(refCompletetionKeySession, (const void*)nullptr, 0))>;//int OnRecv(Iocp::SessionSocketCompeletionKey<WorldSession>& refSession, const char buf[], int len)
		requires std::is_same_v<void, decltype(refSession.OnDestroy())>;//void OnDestroy();
		requires std::is_same_v<void, decltype(refServer.OnAdd(refCompletetionKeySession))>;//void OnAdd(Iocp::SessionSocketCompeletionKey<WorldSession>& session)
	}
bool Iocp::Server<T_Server>::Init(const uint16_t usPort)
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
	si.sin_port = htons(usPort);
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

	//if ( !
	ListenSocketCompeletionKey::StartCoRoutine<T_Session, T_Server>(m_hIocp, m_socketAccept, m_Server);
	//)
//{
//	//Clear();
//	//���������
//	WSACleanup();
//	return false;
//}


	return true;
}


template<class T_Server>
bool Iocp::Server< T_Server>::WsaStartup()
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

template<class T_Server>
void Iocp::Server< T_Server>::Stop()
{
	closesocket(this->m_socketAccept);
	this->m_socketAccept = NULL;
	CloseHandle(this->m_hIocp);
}

template<class T_Server>
template<class T_Session>
//	requires requires(Iocp::SessionSocketCompeletionKey<T_Session>& refCompletetionKeySession, T_Session& refSession, T_Server& refServer)
//{
//	requires std::is_same_v<int, decltype(refSession.OnRecv(refCompletetionKeySession, (const char*)nullptr, 0))>;//int OnRecv(Iocp::SessionSocketCompeletionKey<WorldSession>& refSession, const char buf[], int len)
//	requires std::is_same_v<void, decltype(refSession.OnDestroy())>;//void OnDestroy();
//	requires std::is_same_v<void, decltype(refServer.OnAdd(refCompletetionKeySession))>;//void OnAdd(Iocp::SessionSocketCompeletionKey<WorldSession>& session)
//}
bool Iocp::Server<T_Server>::Connect(const wchar_t* szIp, const wchar_t* szPort)
{
	auto client = new Client();
	auto socket = client->Connect(szIp, szPort, m_hIocp);
	if (socket == 0) 
	{
		LOG(ERROR) << "";// std::wstring(szIp) << ":" << std::wstring(szPort);
		return false;
	}
	//��
	const auto iocp = CreateIoCompletionPort((HANDLE)socket, m_hIocp, (ULONG_PTR)0, 0);
	if (iocp != m_hIocp)
	{
		int a = GetLastError();
		LOG(INFO) << "��ɶ˿ڰ�socketʧ��" << a;

		CloseHandle(iocp);
		closesocket(socket);
		//���������
		WSACleanup();
		return false;
	}

	auto pNewCompleteKey = new Iocp::SessionSocketCompeletionKey<T_Session>(socket);
	pNewCompleteKey->StartCoRoutine();
	return true;
}
