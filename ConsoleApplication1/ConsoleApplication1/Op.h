#pragma once
#include <Winsock2.h>
#include"CoRoutine/CoTask.h"
struct Overlapped;
class SocketCompeletionKey;
//struct IOp {
//	virtual void OnComplete(MyOverlapped* pOverlapped, SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) = 0;
//	CoTask<MyOverlapped*> coTask;
//};
//struct OpAccept :public IOp {
//	virtual void OnComplete(MyOverlapped* pOverlapped, SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) override;
//};
//struct OpRecv:public IOp {
//	virtual void OnComplete(MyOverlapped* pOverlapped, SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) override;
//private:
//	bool PostRecv(SocketCompeletionKey* pKey, MyOverlapped* pOverlapped);
//};
//struct OpSend:public IOp {
//	virtual void OnComplete(MyOverlapped* pOverlapped, SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr) override;
//};