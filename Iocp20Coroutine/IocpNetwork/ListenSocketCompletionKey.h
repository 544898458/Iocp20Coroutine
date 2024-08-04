#pragma once
#include"SessionSocketCompletionKey.h"
namespace Iocp {
	class ListenSocketCompletionKey //:public SocketCompeletionKey
	{
	public:
		//ListenSocketCompletionKey(SOCKET s) :SocketCompeletionKey(s)
		//{

		//}
		template<class T_Session, class T_Server >
			requires requires(Iocp::SessionSocketCompletionKey<T_Session>& refCompletionKeySession, T_Session& refSession, T_Server& refServer)
		{
			requires std::is_same_v<void, decltype(refSession.OnInit(refCompletionKeySession, refServer))>;//void OnInit(Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>& refSession, T_Server&);
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