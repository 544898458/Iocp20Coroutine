#include "Op.h"
#include"SocketCompeletionKey.h"
#include<stdio.h>
void OpAccept::OnComplete(MyOverlapped* pOverlapped, SocketCompeletionKey* pKey, const  HANDLE port, const DWORD      number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
{
	if (!bGetQueuedCompletionStatusReturn)
	{
		switch (lastErr)
		{
		case ERROR_OPERATION_ABORTED:
			printf("The I/O operation has been aborted because of either a thread exit or an application request.");
			break;
		default:
			break;
		}
	}
	printf("�ص�Accept��ɣ�accept\n");
	//�󶨵���ɶ˿�
	auto pNewCompleteKey = new SessionSocketCompeletionKey(pOverlapped->socket);
	//pNewCompleteKey->socket = ;
	HANDLE hPort1 = CreateIoCompletionPort((HANDLE)pNewCompleteKey->Socket(), port, (ULONG_PTR)pNewCompleteKey, 0);
	if (hPort1 != port)
	{
		int a = GetLastError();
		printf("��������Socket��������ɶ˿�ʧ�ܣ�Error=%d\n", a);
		//closesocket(pKey->socket);// all_socks[count]);
		delete pNewCompleteKey;
		return;
	}
	{
		auto pOverlapped = new MyOverlapped(new OpSend());
		pKey->PostSend( pOverlapped);
	}
	{
		auto pOverlapped = new MyOverlapped(new OpRecv());
		//�¿ͻ���Ͷ��recv
		pKey->PostRecv( pOverlapped);
	}
	//count++;
	pKey->PostAccept(pOverlapped);
}

void OpRecv::OnComplete(MyOverlapped* pOverlapped, SocketCompeletionKey* pKey,const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
{
	pKey->OnCompleteRecv(number_of_bytes,pOverlapped);
}

void OpSend::OnComplete(MyOverlapped* pOverlapped, SocketCompeletionKey* pKey, const HANDLE port,const DWORD      number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
{
}
