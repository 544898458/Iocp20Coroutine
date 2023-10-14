#pragma once
#include<WinSock2.h>
#include <mswsock.h>

#include"Define.h"
#include"MyOverlapped.h"
#include<stdio.h>
#include<assert.h>
#include"CoTask.h"
/// <summary>
	/// ��Ӧһ��Socket�������Ǽ���SocketҲ����������Socket
	/// ����Socketͬʱֻ��һ���ص�����Accept
	/// ����Socketͬʱ�������ص�����Recv��Accept
	/// </summary>
class SocketCompeletionKey 
{
public:
	SocketCompeletionKey(SOCKET s) :socket(s) {

	}
	virtual void StartCoRoutine() = 0;
protected:
	SOCKET Socket() const { return socket; }
private:
	virtual bool PostSend( MyOverlapped* pOverlapped) = 0;
	virtual bool PostRecv(MyOverlapped* pOverlapped) = 0;
	//virtual void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped) = 0;
public:
	HANDLE hIocp = nullptr;
private:
	SOCKET socket;
	
};


class SessionSocketCompeletionKey :public SocketCompeletionKey
{
public:
	SessionSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
	{

	}
	//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
	//{
	//	assert(!"SessionSocketCompeletionKey����PostAccept");
	//	return false;
	//}
	virtual void StartCoRoutine() override
	{
		{
			auto pOverlapped = new MyOverlapped();
			PostSend(pOverlapped);
		}
		{
			auto pOverlapped = new MyOverlapped();
			//�¿ͻ���Ͷ��recv
			PostRecv(pOverlapped);
		}
		return ;
	}
	virtual bool PostSend(MyOverlapped* pOverlapped)override
	{
		WSABUF wsabuf;
		wsabuf.buf = (CHAR*)"���";
		wsabuf.len = MAX_RECV_COUNT;

		DWORD dwSendCount;
		DWORD dwFlag = 0;
		int nRes = WSASend(Socket(), &wsabuf, 1, &dwSendCount, dwFlag, &pOverlapped->overlapped, NULL);

		int a = WSAGetLastError();
		if (a == 0)
		{
			//ͬ���������
			PostSend(pOverlapped);
			return true;
		}
		if (ERROR_IO_PENDING != a)
		{
			//�ӳٴ���
			//����ִ�г���
			switch (a) {
			case WSAENOTCONN:
				printf("A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied.\n");
				break;
			}
			return false;
		}
		return true;
	}
	virtual bool PostRecv( MyOverlapped* pOverlapped)override
	{
		WSABUF wsabuf;
		wsabuf.buf = recv_buf;
		wsabuf.len = MAX_RECV_COUNT;

		DWORD dwRecvCount;
		DWORD dwFlag = 0;

		int nRes = WSARecv(Socket(), &wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);

		int a = WSAGetLastError();
		if (ERROR_IO_PENDING == a)
		{
			return true;
		}

		if (a == 0)//ͬ�������ɹ�
		{
			PostRecv(pOverlapped);
			return true;
		}
		printf("PostRecv err");
		return false;
	}
	//void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped)
	//{
	//	if (0 == number_of_bytes)
	//	{
	//		//�ͻ�������
	//		printf("�ͻ������� close\n");
	//		//�ر�
	//		closesocket(Socket());
	//		//WSACloseEvent(all_olp[CompletionKey].hEvent);
	//		//��������ɾ��
	//		//all_socks[CompletionKey] = 0;
	//		//all_olp[CompletionKey].hEvent = NULL;
	//	}
	//	else
	//	{
	//		//if (0 != recv_buf[0])
	//		{
	//			//�յ�  recv
	//			printf("�ص�������ɣ�Recv,%s\n", recv_buf);
	//			memset(recv_buf, 0, sizeof(recv_buf));
	//			//
	//			PostRecv(pOverlapped);
	//		}
	//		//else
	//		//{
	//		//	//send
	//		//	printf("send ok\n");
	//		//}
	//	}
	//}
private:
	char recv_buf[MAX_RECV_COUNT];

};