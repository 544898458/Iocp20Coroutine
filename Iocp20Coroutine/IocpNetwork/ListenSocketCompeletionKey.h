#pragma once
#include"SocketCompeletionKey.h"
namespace Iocp {
	class ListenSocketCompeletionKey //:public SocketCompeletionKey
	{
	public:
		//ListenSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
		//{

		//}
		template<class T_Session, class T_Server >
		static void StartCoRoutine( HANDLE hIocp, SOCKET socketListen, T_Server& refServer);
		//Overlapped acceptOverlapped;
	private:
		template<class T_Session, class T_Server >
		static CoTask<int> PostAccept(Overlapped* pAcceptOverlapped, HANDLE hIocp, SOCKET socketListen, T_Server& refServer);
		static std::tuple<bool,bool> AcceptEx(Overlapped* pAcceptOverlapped, SOCKET socketListen);
		
	};
}