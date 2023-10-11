#pragma once
#include"Op.h"
/// <summary>
/// 重叠操作，对应一次Accet、Recv、Send
/// CONTAINING_RECORD（）可以根据Overlapped获取PER_IO_CONTEXT数据类型。前提是Overlapped是PER_IO_CONTEXT的第一个成员。CONTAINING_RECORD可以根据Overlapped的地址获取其所在PER_IO_CONTEXT结构体的地址指针。
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
	void OnComplete(MyCompeletionKey* pKey, HANDLE port,DWORD      number_of_bytes)
	{
		pOp->OnComplete(this, pKey, port,number_of_bytes);
	}
private:
	IOp* pOp;
};
