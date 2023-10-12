#pragma once
#include <Winsock2.h>

struct MyOverlapped;
struct MyCompeletionKey;
struct IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) = 0;
};
struct OpAccept :public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) override;
};
struct OpRecv:public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) override;
private:
	bool PostRecv(MyCompeletionKey* pKey, MyOverlapped* pOverlapped);
};
struct OpSend:public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) override;
};