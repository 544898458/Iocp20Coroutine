#include "Op.h"
#include"MyCompeletionKey.h"
#include<stdio.h>
inline void OpAccept::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port)
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
		auto pOverlapped = new MyOverlapped(MyOverlapped::Recv);
		//�¿ͻ���Ͷ��recv
		pKey->PostRecv(pNewCompleteKey, pOverlapped);
	}
	//count++;
	pKey->PostAccept();
}

void OpRecv::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port)
{
	if (0 == number_of_bytes)
	{
		//�ͻ�������
		printf("�ͻ������� close\n");
		//�ر�
		closesocket(CompletionKey->socket);
		//WSACloseEvent(all_olp[CompletionKey].hEvent);
		//��������ɾ��
		//all_socks[CompletionKey] = 0;
		//all_olp[CompletionKey].hEvent = NULL;
	}
	else
	{
		if (0 != CompletionKey->recv_buf[0])
		{
			//�յ�  recv
			printf("�ص�������ɣ�Recv,%s\n", CompletionKey->recv_buf);
			memset(CompletionKey->recv_buf, 0, sizeof(CompletionKey->recv_buf));
			//
			pThis->PostRecv(CompletionKey, overlapped);
		}
		else
		{
			//send
			printf("send ok\n");
		}
	}
}

bool OpRecv::PostRecv(MyCompeletionKey* pKey, MyOverlapped* pOverlapped)
{
	WSABUF wsabuf;
	wsabuf.buf = pKey->recv_buf;
	wsabuf.len = MAX_RECV_COUNT;

	DWORD dwRecvCount;
	DWORD dwFlag = 0;

	int nRes = WSARecv(pKey->socket, &wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING == a)
	{
		return true;
	}

	if (a == 0)//ͬ�������ɹ�
	{
		PostRecv(pKey, pOverlapped);
		return true;
	}
	printf("PostRecv err");
	return false;
}
void OpSend::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port)
{
}
