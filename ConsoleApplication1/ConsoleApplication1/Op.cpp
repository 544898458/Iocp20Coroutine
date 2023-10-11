#include "Op.h"
#include"MyCompeletionKey.h"
#include<stdio.h>
void OpAccept::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port,DWORD      number_of_bytes)
{
	printf("�ص�Accept��ɣ�accept\n");
	//�󶨵���ɶ˿�
	auto pNewCompleteKey = new MyCompeletionKey();
	pNewCompleteKey->socket = pOverlapped->socket;
	HANDLE hPort1 = CreateIoCompletionPort((HANDLE)pNewCompleteKey->socket, port, (ULONG_PTR)pNewCompleteKey, 0);
	if (hPort1 != port)
	{
		int a = GetLastError();
		printf("��������Socket��������ɶ˿�ʧ�ܣ�Error=%d\n", a);
		closesocket(pKey->socket);// all_socks[count]);
		delete pNewCompleteKey;
		return;
	}
	{
		auto pOverlapped = new MyOverlapped(new OpSend());
		pKey->PostSend(pNewCompleteKey, pOverlapped);
	}
	{
		auto pOverlapped = new MyOverlapped(new OpRecv());
		//�¿ͻ���Ͷ��recv
		pKey->PostRecv(pNewCompleteKey, pOverlapped);
	}
	//count++;
	pKey->PostAccept();
}

void OpRecv::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port, DWORD      number_of_bytes)
{
	if (0 == number_of_bytes)
	{
		//�ͻ�������
		printf("�ͻ������� close\n");
		//�ر�
		closesocket(pKey->socket);
		//WSACloseEvent(all_olp[CompletionKey].hEvent);
		//��������ɾ��
		//all_socks[CompletionKey] = 0;
		//all_olp[CompletionKey].hEvent = NULL;
	}
	else
	{
		if (0 != pKey->recv_buf[0])
		{
			//�յ�  recv
			printf("�ص�������ɣ�Recv,%s\n", pKey->recv_buf);
			memset(pKey->recv_buf, 0, sizeof(pKey->recv_buf));
			//
			pKey->PostRecv(pKey, pOverlapped);
		}
		else
		{
			//send
			printf("send ok\n");
		}
	}
}

void OpSend::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port,DWORD      number_of_bytes)
{
}
