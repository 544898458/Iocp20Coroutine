#include "SessionSocketCompeletionKey.h"
namespace Iocp {
	template<class T_Session>
	SessionSocketCompeletionKey<T_Session>::~SessionSocketCompeletionKey()
	{
	}
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
			//pSendOverlapped = new Overlapped();
			sendOverlapped.coTask = PostSend(sendOverlapped);
			sendOverlapped.coTask.Run();
		}
		{
			//pRecvOverlapped = new Overlapped();
			recvOverlapped.coTask = PostRecv(recvOverlapped);
			recvOverlapped.coTask.Run();
		}
		return;
	}
	template<class T_Session>
	void SessionSocketCompeletionKey<T_Session>::Send(const char buf[], int len)
	{
		if (this->sendOverlapped.coTask.Finished())
			return;

		this->sendBuf.queue.Enqueue(buf, len);
		this->sendOverlapped.coTask.Run();
	}
	template<class T_Session>
	bool SessionSocketCompeletionKey<T_Session>::Finished()
	{
		std::lock_guard lock(lockFinish);
		return recvFinish && sendFinish;
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostRecv(Overlapped& pOverlapped)
	{
		printf("����PostRecv,this=%d,ThreadId=%d\n",this,GetCurrentThreadId());
		while (true)
		{
			if (!WSARecv(pOverlapped))
			{
				printf("���ܶ�����,���ٵ���WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}

			printf("\n׼���첽�ȴ�WSARecv���\n");
			co_yield 0;
			printf("���첽�ȵ�WSARecv���,numberOfBytesTransferred=%d,GetLastErrorReturn=%d\n",
				pOverlapped.numberOfBytesTransferred, pOverlapped.GetLastErrorReturn);
			if (0 == pOverlapped.numberOfBytesTransferred)
			{
				printf("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSARecv");
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			char* buf(nullptr);
			int len(0);
			std::tie(buf, len) = this->recvBuf.Complete(pOverlapped.numberOfBytesTransferred);
			//this->sendBuf.Enqueue("asdf", 5);
			//this->pSendOverlapped->coTask.Run();
			this->recvBuf.PopFront(this->Session.OnRecv(*this, buf, len));//�ص��û��Զ��庯���������Ǵ����ݣ���������û�У������WebSocketЭ�鸺�������
		}
		//this->pSendOverlapped.coTask.Run();
		//if (!this->sendOverlapped.coTask.Finished())
		{
			std::lock_guard lock(lockFinish);
			recvFinish = true;
			if (!sendFinish)
			{
				printf("PostRecvЭ�̽���������sendOverlapped��û����,GetCurrentThreadId=%d\n", GetCurrentThreadId());
				co_return;
			}
		}
		//this->Session.OnDestroy();
		//delete this;
		printf("PostRecvЭ�̽���,GetCurrentThreadId=%d\n", GetCurrentThreadId());
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostSend(Overlapped& pOverlapped)
	{
		while (true)
		{
			bool needYield, callSend;
			std::tie(needYield, callSend) = WSASend(pOverlapped);
			if (!needYield)
			{
				printf("���ܶ�����,���ٵ���WSASend");
				//delete pOverlapped;
				break;
			}
			if (callSend)
				printf("׼���첽�ȴ�WSASend���,GetThreadId=%d\n", GetCurrentThreadId());
			else
				printf("���������ٷ�WSASend,GetThreadId=%d\n", GetCurrentThreadId());

			printf("��ʼ�첽��WSASend���,pOverlapped.numberOfBytesTransferred=%d,callSend=%d,wsabuf.len=%d,GetLastErrorReturn=%d,GetThreadId=%d\n",
				pOverlapped.numberOfBytesTransferred, callSend, pOverlapped.wsabuf.len, pOverlapped.GetLastErrorReturn, GetCurrentThreadId());
			co_yield 0;
			printf("���첽�ȵ�WSASend���,pOverlapped.numberOfBytesTransferred=%d,callSend=%d,wsabuf.len=%d,GetLastErrorReturn=%d,GetThreadId=%d\n",
				pOverlapped.numberOfBytesTransferred, callSend, pOverlapped.wsabuf.len, pOverlapped.GetLastErrorReturn, GetCurrentThreadId());

			if (!callSend)
			{
				printf("�������ˣ�׼����WSASend\n");
				continue;
			}

			if (0 == pOverlapped.numberOfBytesTransferred && pOverlapped.GetLastErrorReturn != ERROR_IO_PENDING)
			{
				printf("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSASend,pOverlapped.GetLastErrorReturn=%d,GetThreadId=%d\n", pOverlapped.GetLastErrorReturn, GetCurrentThreadId());
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			//if (pOverlapped.GetLastErrorReturn == ERROR_IO_PENDING) 
			//{
			//	printf("ERROR_IO_PENDING��û���꣬�´λ�Ҫ���ŷ�\n");
			//}

			this->sendBuf.Complete(pOverlapped.numberOfBytesTransferred);
		}

		//if (!this->recvOverlapped.coTask.Finished())

		{
			std::lock_guard lock(lockFinish);
			sendFinish = true;
			if (!recvFinish)
			{
				printf("PostSendЭ�̽���������recvOverlapped��û����,GetCurrentThreadId=%d\n", GetCurrentThreadId());
				co_return;
			}
		}

		//this->Session.OnDestroy();
		//delete this;
		printf("PostSendЭ�̽���,GetCurrentThreadId=%d\n", GetCurrentThreadId());
	}

	template<class T_Session>
	bool SessionSocketCompeletionKey<T_Session>::WSARecv(Overlapped& pOverlapped)
	{

		DWORD dwRecvCount(0);
		DWORD dwFlag(0);
		std::tie(pOverlapped.wsabuf.buf, pOverlapped.wsabuf.len) = this->recvBuf.BuildRecvBuf();
		//pOverlapped.GetQueuedCompletionStatusReturn
		printf("����::WSARecv\n");
		const auto recvRet = ::WSARecv(Socket(), &pOverlapped.wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped.overlapped, NULL);
		//pOverlapped.GetLastErrorReturn 
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
	std::tuple<bool, bool>  SessionSocketCompeletionKey<T_Session>::WSASend(Overlapped& pOverlapped)
	{
		DWORD dwFlag = 0;
		pOverlapped.numberOfBytesTransferred = 0;
		pOverlapped.dwSendCount = 0;
		std::tie(pOverlapped.wsabuf.buf, pOverlapped.wsabuf.len) = this->sendBuf.BuildSendBuf();
		if (nullptr == pOverlapped.wsabuf.buf)
		{
			return std::make_tuple(true, false);
		}
		//pOverlapped.GetQueuedCompletionStatusReturn
		int sendRet = ::WSASend(Socket(), &pOverlapped.wsabuf, 1, &pOverlapped.dwSendCount, dwFlag, &pOverlapped.overlapped, NULL);
		//pOverlapped.GetLastErrorReturn
		int err = WSAGetLastError();
		//pOverlapped.numberOfBytesTransferred = dwSendCount;
		if (0 == sendRet)
		{
			printf("WSASend�ص��Ĳ����ɹ�������WSAGetLastError=%d", err);
			return std::make_tuple(true, true);//���δ�����κδ��󣬲��ҷ��Ͳ�����������ɣ� �� WSASend �����㡣 ����������£�һ�������̴߳��ڿɾ���״̬�����Ѽƻ�����������̡�
		}
		if (SOCKET_ERROR != sendRet ||
			ERROR_IO_PENDING != err)
		{
			printf("WSASend�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��%d", pOverlapped.GetLastErrorReturn);
			closesocket(Socket());
			return std::make_tuple(false, true);// �κ�����������붼ָʾ�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��
		}

		// ���򣬽����� ֵ SOCKET_ERROR �����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣 
		// ������� WSA_IO_PENDING ָʾ�ص������ѳɹ��������Ժ�ָʾ��ɡ� 
		return std::make_tuple(true, true);
	}
}