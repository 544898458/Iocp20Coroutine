#include "Op.h"
#include"MyCompeletionKey.h"
#include<stdio.h>
void OpAccept::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port,DWORD      number_of_bytes)
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
		auto pOverlapped = new MyOverlapped(new OpRecv());
		//新客户端投递recv
		pKey->PostRecv(pNewCompleteKey, pOverlapped);
	}
	//count++;
	pKey->PostAccept();
}

void OpRecv::OnComplete(MyOverlapped* pOverlapped, MyCompeletionKey* pKey, HANDLE port, DWORD      number_of_bytes)
{
	if (0 == number_of_bytes)
	{
		//客户端下线
		printf("客户端下线 close\n");
		//关闭
		closesocket(pKey->socket);
		//WSACloseEvent(all_olp[CompletionKey].hEvent);
		//从数组中删掉
		//all_socks[CompletionKey] = 0;
		//all_olp[CompletionKey].hEvent = NULL;
	}
	else
	{
		if (0 != pKey->recv_buf[0])
		{
			//收到  recv
			printf("重叠接受完成：Recv,%s\n", pKey->recv_buf);
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
