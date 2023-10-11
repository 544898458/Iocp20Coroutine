#include "Op.h"
#include"MyCompeletionKey.h"
#include<stdio.h>
inline void OpAccept::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port)
{
	printf("重叠Accept完成：accept\n");
	//绑定到完成端口
	auto pNewCompleteKey = new MyCompeletionKey();
	pNewCompleteKey->socket = pOverlapped->socket;
	HANDLE hPort1 = CreateIoCompletionPort((HANDLE)pNewCompleteKey->socket, port, (ULONG_PTR)pNewCompleteKey, 0);
	if (hPort1 != port)
	{
		int a = GetLastError();
		printf("连上来的Socket关联到完成端口失败，Error=%d\n", a);
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
		//新客户端投递recv
		pKey->PostRecv(pNewCompleteKey, pOverlapped);
	}
	//count++;
	pKey->PostAccept();
}

void OpRecv::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port)
{
	if (0 == number_of_bytes)
	{
		//客户端下线
		printf("客户端下线 close\n");
		//关闭
		closesocket(CompletionKey->socket);
		//WSACloseEvent(all_olp[CompletionKey].hEvent);
		//从数组中删掉
		//all_socks[CompletionKey] = 0;
		//all_olp[CompletionKey].hEvent = NULL;
	}
	else
	{
		if (0 != CompletionKey->recv_buf[0])
		{
			//收到  recv
			printf("重叠接受完成：Recv,%s\n", CompletionKey->recv_buf);
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

	if (a == 0)//同步操作成功
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
