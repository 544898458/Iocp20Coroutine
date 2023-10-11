#pragma once
struct MyOverlapped;
struct MyCompeletionKey;
struct IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port) = 0;
};
struct OpAccept :public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port) override;
};
struct OpRecv:public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port) override;
private:
	bool PostRecv(MyCompeletionKey* pKey, MyOverlapped* pOverlapped);
};
struct OpSend:public IOp {
	virtual void OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port) override;
};