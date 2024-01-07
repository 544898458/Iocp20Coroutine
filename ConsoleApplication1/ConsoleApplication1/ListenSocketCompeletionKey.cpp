#include "ListenSocketCompeletionKey.h"
#include "SessionSocketCompeletionKey.h"
namespace Iocp {
	template<class T_Session>
	void ListenSocketCompeletionKey<T_Session>::StartCoRoutine()
	{
		auto pAcceptOverlapped = new Overlapped();
		pAcceptOverlapped->coTask = PostAccept(pAcceptOverlapped);
		pAcceptOverlapped->coTask.Run();
	}
	template<class T_Session>
	bool ListenSocketCompeletionKey<T_Session>::AcceptEx(Overlapped* pAcceptOverlapped)
	{
		char str2[1];// = new char[1024];// = { 0 };
		DWORD dwRecvcount = 0;
		//int b = WSAGetLastError();
		//OVERLAPPED* Overlapped = new OVERLAPPED();
		pAcceptOverlapped->GetQueuedCompletionStatusReturn = ::AcceptEx(
			this->Socket(),	//[in]�����׽��֡�������Ӧ�ó���������׽����ϵȴ����ӡ�
			pAcceptOverlapped->socket,	//[in]���������ӵ��׽��֡����׽��ֱ��벻���Ѿ��󶨻����Ѿ����ӡ�
			str2, //[in]ָ��һ�����������û��������ڽ����½����ӵ����������ݵĵ�һ���顢�÷������ı��ص�ַ�Ϳͻ��˵�Զ�̵�ַ�����յ������ݽ���д�뵽������0ƫ�ƴ�������ַ���д�롣 �ò�������ָ��������˲�������ΪNULL��������õ�ִ�У�Ҳ�޷�ͨ��GetAcceptExSockaddrs������ñ��ػ�Զ�̵ĵ�ַ��
			0,	//[in]lpOutputBuffer�ֽ�����ָ���������ݻ�����lpOutputBuffer�Ĵ�С����һ��СӦ�������������ı��ص�ַ�Ĵ�С��ͻ��˵�Զ�̵�ַ�����Ǳ�׷�ӵ���������������dwReceiveDataLength���㣬AcceptEx�����ȴ������κ����ݣ����Ǿ��콨�����ӡ�
			sizeof(struct sockaddr_in) + 16,//[in]Ϊ���ص�ַ��Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ�
			sizeof(struct sockaddr_in) + 16,//[in]ΪԶ�̵�ַ����Ϣ�������ֽ�������ֵ��������ô���Э�������ַ��С��16���ֽڡ� ��ֵ����Ϊ0��
			&dwRecvcount,//[out]ָ��һ��DWORD���ڱ�ʶ���յ����ֽ������˲���ֻ����ͬ��ģʽ�������塣�����������ERROR_IO_PENDING���ڳ�Щʱ����ɲ�������ô���DWORDû�����壬��ʱ������ô����֪ͨ�����ж�ȡ�����ֽ�����
			&pAcceptOverlapped->overlapped//overlapped�������ڴ�������㣬����WSAGetLastError�᷵��ERROR_INVALID_HANDLE
		);
		pAcceptOverlapped->GetLastErrorReturn = WSAGetLastError();
		pAcceptOverlapped->numberOfBytesTransferred = dwRecvcount;
		if (pAcceptOverlapped->GetQueuedCompletionStatusReturn)//ͬ������
		{
			assert(0 == pAcceptOverlapped->GetLastErrorReturn);
			return true;//AcceptEx(pAcceptOverlapped);
		}

		if (pAcceptOverlapped->GetLastErrorReturn != ERROR_IO_PENDING)
		{
			printf("AcceptEx err=%d", pAcceptOverlapped->GetLastErrorReturn);
			switch (pAcceptOverlapped->GetLastErrorReturn)
			{
			default:
				printf("AcceptEx err=%d", pAcceptOverlapped->GetLastErrorReturn);
				break;
			}

		}

		return false;
	}
	template<class T_Session>
	CoTask<int> ListenSocketCompeletionKey<T_Session>::PostAccept(Overlapped* pAcceptOverlapped)
	{
		while (true)
		{
			pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (AcceptEx(pAcceptOverlapped))
			{
				printf("ͬ��AcceptEx���\n");
			}
			else
			{
				printf("׼���첽�ȴ��ص�AcceptEx���\n");
				co_yield 0;
				printf("�첽�ص�AcceptEx���\n");
			}

			if (!pAcceptOverlapped->GetQueuedCompletionStatusReturn)
			{
				switch (pAcceptOverlapped->GetLastErrorReturn)
				{
				case ERROR_OPERATION_ABORTED:
					printf("һ����Overlappedû��������ġ�The I/O operation has been aborted because of either a thread exit or an application request.");
					break;
				default:
					break;
				}
			}

			//�󶨵���ɶ˿�
			auto pNewCompleteKey = new SessionSocketCompeletionKey<T_Session>(pAcceptOverlapped->socket);
			pNewCompleteKey->Session.OnInit(*pNewCompleteKey);//�ص��û��Զ��庯��
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
}