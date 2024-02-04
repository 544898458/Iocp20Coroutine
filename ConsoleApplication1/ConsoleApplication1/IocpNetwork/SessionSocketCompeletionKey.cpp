#include <glog/logging.h>

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
			sendOverlapped.coTask.desc = "PostSend";
			sendOverlapped.coTask.Run();
		}
		{
			//pRecvOverlapped = new Overlapped();
			recvOverlapped.coTask = PostRecv(recvOverlapped);
			recvOverlapped.coTask.desc = "PostRecv";
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
		this->sendOverlapped.coTask.Run2(this->sendOverlapped.callSend);
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
		LOG(INFO) << "����PostRecv,this=" << this << ",ThreadId=" << GetCurrentThreadId();
		while (true)
		{
			if (!WSARecv(pOverlapped))
			{
				LOG(INFO) << ("���ܶ�����,���ٵ���WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}

			LOG(INFO) << ("\n׼���첽�ȴ�WSARecv���\n");
			co_yield 0;
			LOG(INFO) << "���첽�ȵ�WSARecv���,numberOfBytesTransferred=" << pOverlapped.numberOfBytesTransferred << ",GetLastErrorReturn=" << pOverlapped.GetLastErrorReturn;
			if (0 == pOverlapped.numberOfBytesTransferred)
			{
				LOG(INFO) << ("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSARecv\n");
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
				LOG(INFO) << "PostRecvЭ�̽���������sendOverlapped��û����,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return;
			}
		}
		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostRecvЭ�̽���,GetCurrentThreadId=" << GetCurrentThreadId();
	}
	template<class T_Session>
	CoTask<int> SessionSocketCompeletionKey<T_Session>::PostSend(Overlapped& overlapped)
	{
		while (true)
		{
			bool needYield;
			std::tie(needYield, overlapped.callSend) = WSASend(overlapped);
			if (!needYield)
			{
				LOG(INFO) << ("���ܶ�����,���ٵ���WSASend");
				//delete pOverlapped;
				break;
			}
			if (overlapped.callSend)
				LOG(INFO) << ("׼���첽�ȴ�WSASend���,GetThreadId=%d\n", GetCurrentThreadId());
			else
				LOG(INFO) << ("���������ٷ�WSASend,GetThreadId=%d\n", GetCurrentThreadId());

			LOG(INFO) << "��ʼ�첽��WSASend���,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
				<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len
				<< ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn << ",GetThreadId=GetCurrentThreadId()";
			co_yield 0;
			LOG(INFO) << "���첽�ȵ�WSASend���,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
				<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len << ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn
				<< ",GetThreadId=" << GetCurrentThreadId();

			if (!overlapped.callSend)
			{
				LOG(INFO) << ("�������ˣ�׼����WSASend\n");
				continue;
			}


			if (0 == overlapped.numberOfBytesTransferred && overlapped.GetLastErrorReturn != ERROR_IO_PENDING)
			{
				LOG(INFO) << ("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSASend,pOverlapped.GetLastErrorReturn=%d,GetThreadId=%d\n", overlapped.GetLastErrorReturn, GetCurrentThreadId());
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			//if (pOverlapped.GetLastErrorReturn == ERROR_IO_PENDING) 
			//{
			//	printf("ERROR_IO_PENDING��û���꣬�´λ�Ҫ���ŷ�\n");
			//}

			this->sendBuf.Complete(overlapped.numberOfBytesTransferred);
		}

		//if (!this->recvOverlapped.coTask.Finished())

		{
			std::lock_guard lock(lockFinish);
			sendFinish = true;
			if (!recvFinish)
			{
				LOG(INFO) << "PostSendЭ�̽���������recvOverlapped��û����,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return;
			}
		}

		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostSendЭ�̽���,GetCurrentThreadId=" << GetCurrentThreadId();
	}

	template<class T_Session>
	bool SessionSocketCompeletionKey<T_Session>::WSARecv(Overlapped& pOverlapped)
	{

		DWORD dwRecvCount(0);
		DWORD dwFlag(0);
		std::tie(pOverlapped.wsabuf.buf, pOverlapped.wsabuf.len) = this->recvBuf.BuildRecvBuf();
		//pOverlapped.GetQueuedCompletionStatusReturn
		LOG(INFO) << ("����::WSARecv\n");
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
			LOG(INFO) << "WSARecv�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��" << err;
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
			LOG(INFO) << "WSASend�ص��Ĳ����ɹ�������WSAGetLastError=" << err;
			return std::make_tuple(true, true);//���δ�����κδ��󣬲��ҷ��Ͳ�����������ɣ� �� WSASend �����㡣 ����������£�һ�������̴߳��ڿɾ���״̬�����Ѽƻ�����������̡�
		}
		if (SOCKET_ERROR != sendRet ||
			ERROR_IO_PENDING != err)
		{
			LOG(INFO) << "WSASend�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��" << pOverlapped.GetLastErrorReturn;
			closesocket(Socket());
			return std::make_tuple(false, true);// �κ�����������붼ָʾ�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��
		}

		// ���򣬽����� ֵ SOCKET_ERROR �����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣 
		// ������� WSA_IO_PENDING ָʾ�ص������ѳɹ��������Ժ�ָʾ��ɡ� 
		return std::make_tuple(true, true);
	}
}