#pragma once
#include"SessionSocketCompeletionKey.h"
namespace Iocp {
	class ListenSocketCompeletionKey //:public SocketCompeletionKey
	{
	public:
		//ListenSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
		//{

		//}
		template<class T_Session, class T_Server >
			requires requires(Iocp::SessionSocketCompeletionKey<T_Session>& refCompletetionKeySession, T_Session& refSession, T_Server& refServer)
		{
			requires std::is_same_v<void, decltype(refSession.OnInit(refCompletetionKeySession, refServer))>;//void OnInit(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>& refSession, T_Server&);
		}
		static void StartCoRoutine(HANDLE hIocp, SOCKET socketListen, T_Server& refServer);
		//Overlapped acceptOverlapped;
	private:
		template<class T_Session, class T_Server >
		//requires requires(T_Session& refSession, T_Server& refServer)
		//{
		//	requires std::is_same_v<void, decltype(refSession.OnInit(&refSession, refServer))>;//void MySession::OnInit(WebSocketSession<MySession>* pWsSession, MyServer& server)
		//}
		static CoTask<int> PostAccept(Overlapped* pAcceptOverlapped, HANDLE hIocp, SOCKET socketListen, T_Server& refServer);
		static std::tuple<bool, bool> AcceptEx(Overlapped* pAcceptOverlapped, SOCKET socketListen);

	};
}