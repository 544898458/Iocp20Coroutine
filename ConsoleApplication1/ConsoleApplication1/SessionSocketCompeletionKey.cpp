#include "SessionSocketCompeletionKey.h"
namespace Iocp {
	//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
	//{
	//	assert(!"SessionSocketCompeletionKey����PostAccept");
	//	return false;
	//}
	template<class T_Session>
	void SessionSocketCompeletionKey<T_Session>::StartCoRoutine()
	{
		{
			//auto pOverlapped = new MyOverlapped();
			//PostSend(pOverlapped);
			pSendOverlapped = new Overlapped();
			pSendOverlapped->coTask = PostSend(pSendOverlapped);
			pSendOverlapped->coTask.Run();
		}
		{
			pRecvOverlapped = new Overlapped();
			pRecvOverlapped->coTask = PostRecv(pRecvOverlapped);
			pRecvOverlapped->coTask.Run();
		}
		return;
	}
	template<class T_Session>
	void SessionSocketCompeletionKey<T_Session>::Send(const char buf[], int len)
	{
		this->sendBuf.queue.Enqueue(buf, len);
		this->pSendOverlapped->coTask.Run();
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostRecv(Overlapped* pOverlapped)
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
			char* buf(nullptr);
			int len(0);
			std::tie(buf, len) = this->recvBuf.Complete(pOverlapped->numberOfBytesTransferred);
			//this->sendBuf.Enqueue("asdf", 5);
			//this->pSendOverlapped->coTask.Run();
			this->recvBuf.PopFront(this->Session.OnRecv(*this, buf, len));
		}
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostSend(Overlapped* pOverlapped)
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
			printf("���첽�ȵ�WSASend���,pOverlapped->numberOfBytesTransferred=%d,callSend=%d\n", pOverlapped->numberOfBytesTransferred, callSend);

			if (!callSend)
			{
				printf("�����ݿɷ�����������ʱ�ٵ���WSASend\n");
				continue;
			}

			if (0 == pOverlapped->numberOfBytesTransferred)
			{
				printf("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSASend\n");
				CloseSocket();
				delete pOverlapped;
				break;
			}

			this->sendBuf.Complete(pOverlapped->numberOfBytesTransferred);
			co_return 0;
		}
	}

	template<class T_Session>
	bool SessionSocketCompeletionKey<T_Session>::WSARecv(Overlapped* pOverlapped)
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
	template<class T_Session>
	std::tuple<bool, bool>  SessionSocketCompeletionKey<T_Session>::WSASend(Overlapped* pOverlapped)
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
}