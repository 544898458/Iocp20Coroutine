#pragma once
#include"Op.h"
/// <summary>
/// �ص���������Ӧһ��Accet��Recv��Send
/// CONTAINING_RECORD�������Ը���Overlapped��ȡPER_IO_CONTEXT�������͡�ǰ����Overlapped��PER_IO_CONTEXT�ĵ�һ����Ա��CONTAINING_RECORD���Ը���Overlapped�ĵ�ַ��ȡ������PER_IO_CONTEXT�ṹ��ĵ�ַָ�롣
///	PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(lpOverlapped, PER_IO_CONTEXT, m_Overlapped);
/// </summary>
struct MyOverlapped
{
	//enum Op
	//{
	//	Accept,
	//	Recv,
	//	Send,
	//};
	MyOverlapped(IOp* opInit)
	{
		this->pOp = opInit;
	}
	OVERLAPPED overlapped = { 0 };

	
	SOCKET socket;
	void OnComplete(MyCompeletionKey* pKey, HANDLE port)
	{
		pOp->OnComplete(this, pKey, port);
	}
private:
	IOp* pOp;
};
