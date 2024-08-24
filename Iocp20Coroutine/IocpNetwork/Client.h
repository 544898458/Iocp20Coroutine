#pragma once

#include <WinSock2.h>
#include <stdint.h>
#include "SessionSocketCompletionKey.h"
namespace Iocp
{
	class Client
	{
	public:
		template<class T_Session>
		//	requires requires(Iocp::SessionSocketCompletionKey<T_Session>& refCompletetionKeySession, T_Session& refSession, T_Server& refServer)
		//{
		//	requires std::is_same_v<int, decltype(refSession.OnRecv(refCompletetionKeySession, (const char*)nullptr, 0))>;//int OnRecv(Iocp::SessionSocketCompletionKey<WorldSession>& refSession, const char buf[], int len)
		//	requires std::is_same_v<void, decltype(refSession.OnDestroy())>;//void OnDestroy();
		//	requires std::is_same_v<void, decltype(refServer.OnAdd(refCompletetionKeySession))>;//void OnAdd(Iocp::SessionSocketCompletionKey<WorldSession>& session)
		//}
		static Iocp::SessionSocketCompletionKey<T_Session>* Connect(const wchar_t* szIp, const wchar_t* szPort, HANDLE hIocp);
		SOCKET Connect(const wchar_t* szIp, const wchar_t* szPort);
	};
}

template<class T_Session>
//	requires requires(Iocp::SessionSocketCompletionKey<T_Session>& refCompletetionKeySession, T_Session& refSession, T_Server& refServer)
//{
//	requires std::is_same_v<int, decltype(refSession.OnRecv(refCompletetionKeySession, (const char*)nullptr, 0))>;//int OnRecv(Iocp::SessionSocketCompletionKey<WorldSession>& refSession, const char buf[], int len)
//	requires std::is_same_v<void, decltype(refSession.OnDestroy())>;//void OnDestroy();
//	requires std::is_same_v<void, decltype(refServer.OnAdd(refCompletetionKeySession))>;//void OnAdd(Iocp::SessionSocketCompletionKey<WorldSession>& session)
//}
Iocp::SessionSocketCompletionKey<T_Session>* Iocp::Client::Connect(const wchar_t* szIp, const wchar_t* szPort, HANDLE m_hIocp)
{
	auto client = new Client();
	auto socket = client->Connect(szIp, szPort);
	if (socket == 0)
	{
		LOG(ERROR) << "";// std::wstring(szIp) << ":" << std::wstring(szPort);
		return nullptr;
	}
	//绑定
	const auto iocp = CreateIoCompletionPort((HANDLE)socket, m_hIocp, (ULONG_PTR)0, 0);
	if (iocp != m_hIocp)
	{
		int a = GetLastError();
		LOG(WARNING) << "完成端口绑定socket失败" << a;

		CloseHandle(iocp);
		closesocket(socket);
		//清理网络库
		WSACleanup();
		return nullptr;
	}

	auto pNewCompleteKey = new Iocp::SessionSocketCompletionKey<T_Session>(socket);
	pNewCompleteKey->StartCoRoutine();
	return pNewCompleteKey;
}
