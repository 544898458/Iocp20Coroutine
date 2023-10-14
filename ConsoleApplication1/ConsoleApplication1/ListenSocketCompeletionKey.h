#pragma once
#include"SocketCompeletionKey.h"


class ListenSocketCompeletionKey :public SocketCompeletionKey
{
public:
	ListenSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
	{

	}
	virtual void StartCoRoutine() override;

private:
	CoTask<int> PostAccept(MyOverlapped* pAcceptOverlapped);
	bool AcceptEx(MyOverlapped* pAcceptOverlapped);
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
