#include "Server.h"
#include "ListenSocketCompeletionKey.h"
/// <summary>
/// 
/// </summary>
/// <typeparam name="T_Session"></typeparam>
/// <returns></returns>
template<class T_Server>
template<class T_Session>
	requires requires(T_Session& refSession)
{
	//std::is_function_v<decltype(T_Session::OnInit)>;
	std::is_function_v<decltype(T_Session::OnRecv)>;
	std::is_function_v<decltype(T_Session::OnDestroy)>;
}
bool Iocp::Server<T_Server>::Init()
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

	//auto pListenCompleteKey = new ListenSocketCompeletionKey<T_Session>(this->socketAccept);
	//创建完成端口	创建一个I/O完成端口对象，用它面向任意数量的套接字句柄，管理多个I/O请求。要做到这一点，需要调用CreateCompletionPort函数。
	this->m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == m_hIocp)
	{
		int a = GetLastError();
		LOG(INFO) << a;
		closesocket(m_socketAccept);
		//清理网络库
		//WSACleanup();
		return false;
	}
	//绑定
	const auto iocp = CreateIoCompletionPort((HANDLE)m_socketAccept, m_hIocp, (ULONG_PTR)0, 0);
	if (iocp != m_hIocp)
	{
		int a = GetLastError();
		LOG(INFO) << "完成端口绑定socket失败" << a;

		CloseHandle(iocp);
		closesocket(m_socketAccept);
		//清理网络库
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
		//出错了
		int a = WSAGetLastError();
		const char* szErr = "";
		switch (a) {
		case WSAEADDRNOTAVAIL:szErr = "The requested address is not valid in its context.在其上下文中，该请求的地址无效。"; break;
		}
		LOG(INFO) << "a=" << a << "," << szErr;


		//释放
		closesocket(m_socketAccept);
		//清理网络库
		WSACleanup();
		return false;
	}

	if (SOCKET_ERROR == listen(m_socketAccept, SOMAXCONN))
	{
		//出错了
		int a = WSAGetLastError();
		//释放
		CloseHandle(iocp);
		closesocket(m_socketAccept);
		//清理网络库
		WSACleanup();
		return 0;
	}
	//创建线程数量有了
	SYSTEM_INFO system_processors_count;
	GetSystemInfo(&system_processors_count);
	auto process_count = system_processors_count.dwNumberOfProcessors;
	for (decltype(process_count) i = 0; i < process_count; i++)
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
		//	//清理网络库
		//	//WSACleanup();
		//	return false;
		//}
		//this->vecThread.push_back(hThread);
	}

	//if ( !
	ListenSocketCompeletionKey::StartCoRoutine<T_Session,T_Server>(m_hIocp, m_socketAccept,m_Server);
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
			LOG(INFO) << "重启下电脑试试，或者检查网络库";
			break;
		case WSAVERNOTSUPPORTED:
			LOG(INFO) << ("请更新网络库");
			break;
		case WSAEINPROGRESS:
			LOG(INFO) << ("请重新启动");
			break;
		case WSAEPROCLIM:
			LOG(INFO) << ("请尝试关掉不必要的软件，以为当前网络运行提供充足资源");
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
void Iocp::Server< T_Server>::NetworkThreadProc(HANDLE port)
{
	DWORD      number_of_bytes = 0;
	SocketCompeletionKey* pCompletionKey = nullptr;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		//pCompletionKey对应一个socket，lpOverlapped对应一次事件
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&pCompletionKey, &lpOverlapped, INFINITE);//没完成就会卡在这里，正常
		int lastErr = GetLastError();//可能是Socket强制关闭
		if (lpOverlapped != nullptr)
		{
			auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
			overlapped->OnComplete(pCompletionKey, port, number_of_bytes, bFlag, lastErr);
			if (overlapped->needDeleteMe && overlapped->coTask.Finished())
			{
				LOG(INFO) << "删除" << overlapped->coTask.m_desc;
				delete overlapped;
				overlapped = nullptr;
			}
		}
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
