#pragma once
#include<WinSock2.h>
#include <mswsock.h>

#include"Define.h"
#include"MyOverlapped.h"
#include<stdio.h>
#include<assert.h>
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
	virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)=0;
	virtual bool PostSend( MyOverlapped* pOverlapped) = 0;
	virtual bool PostRecv(MyOverlapped* pOverlapped) = 0;
	virtual void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped) = 0;
	SOCKET Socket() const { return socket; }
private:
	SOCKET socket;
};

class ListenSocketCompeletionKey :public SocketCompeletionKey 
{
public:
	ListenSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s) 
	{

	}
	virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override
	{
		//auto pAcceptOverlapped = new MyOverlapped(new OpAccept());

		pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		//all_olp[count].hEvent = WSACreateEvent();

		char* str2 = new char[1024];// = { 0 };
		DWORD dwRecvcount = 0;
		int b = WSAGetLastError();
		//OVERLAPPED* Overlapped = new OVERLAPPED();
		BOOL bRes = AcceptEx(
			this->Socket(),	//[in]�����׽��֡�������Ӧ�ó���������׽����ϵȴ����ӡ�
			pAcceptOverlapped->socket,	//[in]���������ӵ��׽��֡����׽��ֱ��벻���Ѿ��󶨻����Ѿ����ӡ�
			str2, //[in]ָ��һ�����������û��������ڽ����½����ӵ����������ݵĵ�һ���顢�÷������ı��ص�ַ�Ϳͻ��˵�Զ�̵�ַ�����յ������ݽ���д�뵽������0ƫ�ƴ�������ַ���д�롣 �ò�������ָ��������˲�������ΪNULL��������õ�ִ�У�Ҳ�޷�ͨ��GetAcceptExSockaddrs������ñ��ػ�Զ�̵ĵ�ַ��
			0,	//[in]lpOutputBuffer�ֽ�����ָ���������ݻ�����lpOutputBuffer�Ĵ�С����һ��СӦ�������������ı��ص�ַ�Ĵ�С��ͻ��˵�Զ�̵�ַ�����Ǳ�׷�ӵ���������������dwReceiveDataLength���㣬AcceptEx�����ȴ������κ����ݣ����Ǿ��콨�����ӡ�
			sizeof(struct sockaddr_in) + 16,//[in]Ϊ���ص�ַ��Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ�
			sizeof(struct sockaddr_in) + 16,//[in]ΪԶ�̵�ַ����Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ� ��ֵ����Ϊ0��
			&dwRecvcount,//[out]ָ��һ��DWORD���ڱ�ʶ���յ����ֽ������˲���ֻ����ͬ��ģʽ�������塣�����������ERROR_IO_PENDING���ڳ�Щʱ����ɲ�������ô���DWORDû�����壬��ʱ������ô����֪ͨ�����ж�ȡ�����ֽ�����
			&pAcceptOverlapped->overlapped//overlapped�������ڴ�������㣬����WSAGetLastError�᷵��ERROR_INVALID_HANDLE
		);
		const auto lastErr = WSAGetLastError();
		if (!bRes && lastErr != ERROR_IO_PENDING)
		{
			switch (lastErr)
			{
			default:
				printf("AcceptEx err=%d", lastErr);
				break;
			}
			return false;
		}


		return true;
	}
	virtual bool PostSend(MyOverlapped* pAcceptOverlapped)override
	{
		assert(!"ListenSocketCompeletionKey����PostSend");
		return false;
	}
	virtual bool PostRecv(MyOverlapped* pAcceptOverlapped)override
	{
		assert(!"ListenSocketCompeletionKey����PostRecv");
		return false;
	}
	virtual void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped) override
	{
		assert(!"ListenSocketCompeletionKey����OnCompleteRecv");
	}
};

class SessionSocketCompeletionKey :public SocketCompeletionKey
{
public:
	SessionSocketCompeletionKey(SOCKET s) :SocketCompeletionKey(s)
	{

	}
	virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
	{
		assert(!"SessionSocketCompeletionKey����PostAccept");
		return false;
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
	void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped)
	{
		if (0 == number_of_bytes)
		{
			//�ͻ�������
			printf("�ͻ������� close\n");
			//�ر�
			closesocket(Socket());
			//WSACloseEvent(all_olp[CompletionKey].hEvent);
			//��������ɾ��
			//all_socks[CompletionKey] = 0;
			//all_olp[CompletionKey].hEvent = NULL;
		}
		else
		{
			//if (0 != recv_buf[0])
			{
				//�յ�  recv
				printf("�ص�������ɣ�Recv,%s\n", recv_buf);
				memset(recv_buf, 0, sizeof(recv_buf));
				//
				PostRecv(pOverlapped);
			}
			//else
			//{
			//	//send
			//	printf("send ok\n");
			//}
		}
	}
private:
	char recv_buf[MAX_RECV_COUNT];

};