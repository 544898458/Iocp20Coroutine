#pragma once
#include "SocketCompeletionKey.h"
#include<WinSock2.h>
#include "ByteQueue.h"
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
	virtual void StartCoRoutine() override;
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
	CoTask<int> PostSend(MyOverlapped* pOverlapped);
	CoTask<int> PostRecv(MyOverlapped* pOverlapped);
	std::tuple<bool, bool>  WSASend(MyOverlapped* pOverlapped);
private:
	char recv_buf[MAX_RECV_COUNT] = {};
	ByteQueue sendBuf;
};