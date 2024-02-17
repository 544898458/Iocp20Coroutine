#pragma once
#include"SocketCompeletionKey.h"
namespace Iocp {
	template<class T_Session>
	class ListenSocketCompeletionKey :public SocketCompeletionKey
	{
	public:
		ListenSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
		{

		}
		virtual void StartCoRoutine() override;

	private:
		CoTask<int> PostAccept(Overlapped* pAcceptOverlapped);
		std::tuple<bool,bool> AcceptEx(Overlapped* pAcceptOverlapped);
		//virtual bool PostSend(MyOverlapped* pAcceptOverlapped)override
		//{
		//	assert(!"ListenSocketCompeletionKey����PostSend");
		//	return false;
		//}
		//virtual bool PostRecv(MyOverlapped* pAcceptOverlapped)override
		//{
		//	assert(!"ListenSocketCompeletionKey����PostRecv");
		//	return false;
		//}
		//virtual void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped) override
		//{
		//	assert(!"ListenSocketCompeletionKey����OnCompleteRecv");
		//}
	private:
		//CoTask<MyOverlapped*> taskAccept;
	};
}