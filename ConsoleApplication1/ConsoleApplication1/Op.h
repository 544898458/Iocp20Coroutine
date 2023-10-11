#pragma once
#include <Winsock2.h>

struct MyOverlapped;
struct MyCompeletionKey;
struct IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port, DWORD      number_of_bytes) = 0;
};
struct OpAccept :public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port, DWORD      number_of_bytes) override;
};
struct OpRecv:public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port, DWORD      number_of_bytes) override;
private:
	bool PostRecv(MyCompeletionKey* pKey, MyOverlapped* pOverlapped);
};
struct OpSend:public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port, DWORD      number_of_bytes) override;
};