#pragma once
#include"SocketCompeletionKey.h"
namespace Iocp {
	template<class T_Session>
	class ListenSocketCompeletionKey //:public SocketCompeletionKey
	{
	public:
		//ListenSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
		//{

		//}
		static void StartCoRoutine( HANDLE hIocp, SOCKET socketListen);
		//Overlapped acceptOverlapped;
	private:
		static CoTask<int> PostAccept(Overlapped* pAcceptOverlapped, HANDLE hIocp, SOCKET socketListen);
		static std::tuple<bool,bool> AcceptEx(Overlapped* pAcceptOverlapped, SOCKET socketListen);
		
	};
}