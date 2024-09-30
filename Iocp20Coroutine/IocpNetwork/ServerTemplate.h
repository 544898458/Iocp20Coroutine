#include "Server.h"
#include "ListenSocketCompletionKey.h"
#include "SessionSocketCompletionKey.h"
#include "Client.h"
/// <summary>
/// 
/// </summary>
/// <typeparam name="T_Session"></typeparam>
/// <returns></returns>
template<class T_Server>
template<class T_Session>
	requires requires(Iocp::SessionSocketCompletionKey<T_Session>& refCompletionKeySession, T_Session& refSession, T_Server& refServer)
	{
		requires std::is_same_v<int, decltype(refSession.OnRecv(refCompletionKeySession, (const void*)nullptr, 0))>;//int OnRecv(Iocp::SessionSocketCompletionKey<WorldSession>& refSession, const char buf[], int len)
		requires std::is_same_v<void, decltype(refSession.OnDestroy())>;//void OnDestroy();
		requires std::is_same_v<void, decltype(refServer.OnAdd(refCompletionKeySession))>;//void OnAdd(Iocp::SessionSocketCompletionKey<WorldSession>& session)
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
		//清理网络库
		//WSACleanup();
		return false;
	}

	//auto pListenCompleteKey = new ListenSocketCompletionKey<T_Session>(this->socketAccept);

	//绑定
	const auto iocp = CreateIoCompletionPort((HANDLE)m_socketAccept, m_hIocp, (ULONG_PTR)0, 0);
	if (iocp != m_hIocp)
	{
		int a = GetLastError();
		LOG(WARNING) << "完成端口绑定socket失败" << a;

		CloseHandle(iocp);
		closesocket(m_socketAccept);
		//清理网络库
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
		//出错了
		int a = WSAGetLastError();
		const char* szErr = "";
		switch (a) {
		case WSAEADDRNOTAVAIL:szErr = "The requested address is not valid in its context.在其上下文中，该请求的地址无效。"; break;
		}
		LOG(WARNING) << "a=" << a << "," << szErr;


		//释放
		closesocket(m_socketAccept);
		//清理网络库
		WSACleanup();
		return false;
	}

	if (SOCKET_ERROR == listen(m_socketAccept, SOMAXCONN))
	{
		//出错了
		int err = WSAGetLastError();
		LOG(WARNING) << "err=" << err;
		//释放
		CloseHandle(iocp);
		closesocket(m_socketAccept);
		//清理网络库
		WSACleanup();
		return 0;
	}

	//if ( !
	ListenSocketCompletionKey::StartCoRoutine<T_Session, T_Server>(m_hIocp, m_socketAccept, m_Server);
	//)
//{
//	//Clear();
//	//清理网络库
//	WSACleanup();
//	return false;
//}


	return true;
}

template<class T_Server>
void Iocp::Server< T_Server>::Stop()
{
	LOG(INFO) << "m_socketAccept=" << this->m_socketAccept;
	closesocket(this->m_socketAccept);
	this->m_socketAccept = NULL;
	CloseHandle(this->m_hIocp);
}
