#include "SessionSocketCompeletionKey.h"

//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
//{
//	assert(!"SessionSocketCompeletionKey����PostAccept");
//	return false;
//}

void SessionSocketCompeletionKey::StartCoRoutine()
{
	{
		//auto pOverlapped = new MyOverlapped();
		//PostSend(pOverlapped);
		pSendOverlapped = new MyOverlapped();
		pSendOverlapped->coTask = PostSend(pSendOverlapped);
		pSendOverlapped->coTask.Run();
	}
	{
		pRecvOverlapped = new MyOverlapped();
		pRecvOverlapped->coTask = PostRecv(pRecvOverlapped);
		pRecvOverlapped->coTask.Run();
	}
	return;
}

inline CoTask<int> SessionSocketCompeletionKey::PostRecv(MyOverlapped* pOverlapped)
{
	while (true)
	{
		if (!WSARecv(pOverlapped))
		{
			printf("���ܶ�����,���ٵ���WSARecv\n");
			CloseSocket();
			delete pOverlapped;
			break;
		}

		printf("\n׼���첽�ȴ�WSARecv���\n");
		co_yield 0;
		printf("���첽�ȵ�WSARecv���,numberOfBytesTransferred=%d,GetLastErrorReturn=%d\n",
			pOverlapped->numberOfBytesTransferred, pOverlapped->GetLastErrorReturn);
		if (0 == pOverlapped->numberOfBytesTransferred)
		{
			printf("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSASend");
			CloseSocket();
			delete pOverlapped;
			break;
		}
		this->recvBuf.Complete(pOverlapped->numberOfBytesTransferred);
		this->sendBuf.Enqueue("asdf", 5);
		this->pSendOverlapped->coTask.Run();
	}
}
CoTask<int> SessionSocketCompeletionKey::PostSend(MyOverlapped* pOverlapped)
{
	while (true)
	{
		bool needYield, callSend;
		std::tie(needYield, callSend) = WSASend(pOverlapped);
		if (!needYield)
		{
			printf("���ܶ�����,���ٵ���WSASend");
			delete pOverlapped;
			break;
		}

		printf("׼���첽�ȴ�WSASend���\n");
		co_yield 0;
		printf("���첽�ȵ�WSASend���,pOverlapped->numberOfBytesTransferred=%d\n", pOverlapped->numberOfBytesTransferred);

		if (!callSend)
		{
			printf("�����ݿɷ�����������ʱ�ٵ���WSASend\n");
			continue;
		}

		if (0 == pOverlapped->numberOfBytesTransferred)
		{
			printf("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSASend");
			CloseSocket();
			delete pOverlapped;
			break;
		}

		
		this->sendBuf.Complete(pOverlapped->numberOfBytesTransferred);
	}
}
bool SessionSocketCompeletionKey::WSARecv(MyOverlapped* pOverlapped)
{

	DWORD dwRecvCount(0);
	DWORD dwFlag(0);
	std::tie(pOverlapped->wsabuf.buf, pOverlapped->wsabuf.len) = this->recvBuf.BuildRecvBuf();
	//pOverlapped->GetQueuedCompletionStatusReturn
	const auto recvRet = ::WSARecv(Socket(), &pOverlapped->wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);
	//pOverlapped->GetLastErrorReturn 
	const auto err = WSAGetLastError();

	if (0 == recvRet)//���δ�����κδ��󣬲��ҽ��ղ�����������ɣ� �� WSARecv �����㡣 ����������£��ڵ����̴߳��ڿɾ���״̬�󣬽��Ѽƻ�����������̡�
	{
		return true;
	}

	if (SOCKET_ERROR != recvRet ||
		ERROR_IO_PENDING != err)
	{
		printf("WSARecv�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��%d\n", err);
		return false;// �κ�����������붼ָʾ�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��
	}

	// ���򣬽����� ֵ SOCKET_ERROR �����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣 
	// ������� WSA_IO_PENDING ָʾ�ص������ѳɹ��������Ժ�ָʾ��ɡ� 
	return true;
}

/// <summary>
/// 
/// </summary>
/// <param name="pOverlapped"></param>
/// <param name="refSize"></param>
/// <returns>�����ȴ��첽���,û������Ҫ����û�е���WSASend</returns>
std::tuple<bool, bool>  SessionSocketCompeletionKey::WSASend(MyOverlapped* pOverlapped)
{
	DWORD dwSendCount(0);
	DWORD dwFlag = 0;
	std::tie(pOverlapped->wsabuf.buf, pOverlapped->wsabuf.len) = this->sendBuf.BuildSendBuf();
	if (nullptr == pOverlapped->wsabuf.buf)
	{
		return std::make_tuple(true, false);
	}
	//pOverlapped->GetQueuedCompletionStatusReturn
	int sendRet = ::WSASend(Socket(), &pOverlapped->wsabuf, 1, &dwSendCount, dwFlag, &pOverlapped->overlapped, NULL);
	//pOverlapped->GetLastErrorReturn
	int err = WSAGetLastError();
	//pOverlapped->numberOfBytesTransferred = dwSendCount;
	if (0 == sendRet)
	{
		return std::make_tuple(true, true);//���δ�����κδ��󣬲��ҷ��Ͳ�����������ɣ� �� WSASend �����㡣 ����������£�һ�������̴߳��ڿɾ���״̬�����Ѽƻ�����������̡�
	}
	if (SOCKET_ERROR != sendRet ||
		ERROR_IO_PENDING != err)
	{
		printf("WSASend�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��%d", pOverlapped->GetLastErrorReturn);
		closesocket(Socket());
		return std::make_tuple(false, true);// �κ�����������붼ָʾ�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��
	}

	// ���򣬽����� ֵ SOCKET_ERROR �����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣 
	// ������� WSA_IO_PENDING ָʾ�ص������ѳɹ��������Ժ�ָʾ��ɡ� 
	return std::make_tuple(true, true);
}
