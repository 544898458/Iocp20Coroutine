#pragma once
#include"SocketCompeletionKey.h"
namespace Iocp {
	class ListenSocketCompeletionKey //:public SocketCompeletionKey
	{
	public:
		//ListenSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
		//{

		//}
		template<class T_Session>
		static void StartCoRoutine( HANDLE hIocp, SOCKET socketListen);
		//Overlapped acceptOverlapped;
	private:
		template<class T_Session>
		static CoTask<int> PostAccept(Overlapped* pAcceptOverlapped, HANDLE hIocp, SOCKET socketListen);
		static std::tuple<bool,bool> AcceptEx(Overlapped* pAcceptOverlapped, SOCKET socketListen);
		
	};
}