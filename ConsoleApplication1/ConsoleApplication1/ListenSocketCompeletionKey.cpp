#include "ListenSocketCompeletionKey.h"

void ListenSocketCompeletionKey::StartCoRoutine() 
{
	//auto op = new OpAccept();
	auto pAcceptOverlapped = new MyOverlapped();
	pAcceptOverlapped->coTask = PostAccept(pAcceptOverlapped) ;
	pAcceptOverlapped->coTask.Run();
}
void ListenSocketCompeletionKey::AcceptEx(MyOverlapped* pAcceptOverlapped)
{
	char str2[1];// = new char[1024];// = { 0 };
	DWORD dwRecvcount = 0;
	//int b = WSAGetLastError();
	//OVERLAPPED* Overlapped = new OVERLAPPED();
	BOOL bRes = ::AcceptEx(
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
	if (bRes && 0 == lastErr)//ͬ������
	{
		AcceptEx(pAcceptOverlapped);
	}
	if (!bRes && lastErr != ERROR_IO_PENDING)
	{
		switch (lastErr)
		{
		default:
			printf("AcceptEx err=%d", lastErr);
			break;
		}
		
	}
}
CoTask<MyOverlapped*> ListenSocketCompeletionKey::PostAccept(MyOverlapped* pAcceptOverlapped)
{
	//auto pAcceptOverlapped = new MyOverlapped(new OpAccept());
	
	while (true) {
		pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		//all_olp[count].hEvent = WSACreateEvent();
		AcceptEx(pAcceptOverlapped);
		co_yield pAcceptOverlapped;
		if (!pAcceptOverlapped->bGetQueuedCompletionStatusReturn)
		{
			switch (pAcceptOverlapped->lastErr)
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
		auto pNewCompleteKey = new SessionSocketCompeletionKey(pAcceptOverlapped->socket);
		//pNewCompleteKey->socket = ;
		HANDLE hPort1 = CreateIoCompletionPort((HANDLE)pAcceptOverlapped->socket, this->hIocp, (ULONG_PTR)pNewCompleteKey, 0);
		if (hPort1 != this->hIocp)
		{
			int a = GetLastError();
			printf("��������Socket��������ɶ˿�ʧ�ܣ�Error=%d\n", a);
			//closesocket(pKey->socket);// all_socks[count]);
			delete pNewCompleteKey;
			co_return 0;
		}
		pNewCompleteKey->StartCoRoutine();
	}
}
