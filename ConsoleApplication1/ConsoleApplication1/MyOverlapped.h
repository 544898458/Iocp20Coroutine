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

	OVERLAPPED overlapped = { 0 };

	
	SOCKET socket=NULL;

	//��ɶ˿ڷ��ؽ����GetQueuedCompletionStatus ���ؽ��
	DWORD number_of_bytes;
	BOOL bGetQueuedCompletionStatusReturn;
	int lastErr;

	void OnComplete(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
	{
		this->number_of_bytes = number_of_bytes;
		this->bGetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
		this->lastErr = lastErr;
		this->coTask.Run();
		//pOp->OnComplete(this, pKey, port,number_of_bytes,bGetQueuedCompletionStatusReturn,lastErr);
	}
	CoTask<MyOverlapped*> coTask;
};
