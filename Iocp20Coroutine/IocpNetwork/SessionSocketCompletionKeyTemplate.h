#include <glog/logging.h>

#include "SessionSocketCompletionKey.h"

//template<class T_Session>
//std::set<Iocp::SessionSocketCompletionKey<T_Session>*> g_setSession;
//template<class T_Session>
//std::mutex g_setSessionMutex;

namespace Iocp {
	template<class T_Session>
	SessionSocketCompletionKey<T_Session>::~SessionSocketCompletionKey()
	{
	}
	//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
	//{
	//	assert(!"SessionSocketCompletionKey����PostAccept");
	//	return false;
	//}
	template<class T_Session>
	void SessionSocketCompletionKey<T_Session>::StartCoRoutine(HANDLE hIocp)
	{
		m_hIocp = hIocp;
		{
			//auto pOverlapped = new MyOverlapped();
			//PostSend(pOverlapped);
			//pSendOverlapped = new Overlapped();
			sendOverlapped.OnComplete = &Overlapped::OnCompleteSend;
			sendOverlapped.coTask = PostSend(sendOverlapped);
			sendOverlapped.coTask.m_desc = "PostSend";
			PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &notifySendOverlapped.overlapped);
			//sendOverlapped.coTask.Run();
		}
		{
			//pRecvOverlapped = new Overlapped();
			recvOverlapped.coTask = PostRecv(recvOverlapped);
			recvOverlapped.coTask.m_desc = "PostRecv";
			recvOverlapped.coTask.Run();
		}
		return;
	}
	/// <summary>
	/// ���������̣߳����̣߳�����
	/// </summary>
	/// <typeparam name="T_Session"></typeparam>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	template<class T_Session>
	void SessionSocketCompletionKey<T_Session>::Send(const void* buf, int len)
	{
		if (this->sendOverlapped.coTask.Finished())
		{
			LOG(WARNING) << "Sending Failed";
			return;
		}

		this->sendBuf.queue.Enqueue(buf, len);
		//this->sendOverlapped.coTask.Run2(this->sendOverlapped.callSend);
		{
			//std::lock_guard lock(this->sendOverlapped.coTask.m_mutex);
			if (Overlapped::SendState_Sending != this->sendOverlapped.atomicSendState.load())
			{
				PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &notifySendOverlapped.overlapped);
			}
		}
	}
	template<class T_Session>
	bool SessionSocketCompletionKey<T_Session>::Finished()
	{
		std::lock_guard lock(lockFinish);
		return recvFinish && sendFinish;
	}
	template<class T_Session>
	CoTask<Overlapped::YieldReturn> SessionSocketCompletionKey<T_Session>::PostRecv(Overlapped& pOverlapped)
	{
		LOG(INFO) << "����PostRecv,this=" << this << ",ThreadId=" << GetCurrentThreadId();
		while (true)
		{
			if (!WSARecv(pOverlapped))
			{
				LOG(WARNING) << ("���ܶ�����,���ٵ���WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}

			//LOG(INFO) << ("\n׼���첽�ȴ�WSARecv���\n");
			co_yield Overlapped::OK;
			//LOG(INFO) << "���첽�ȵ�WSARecv���,numberOfBytesTransferred=" << pOverlapped.numberOfBytesTransferred << ",GetLastErrorReturn=" << pOverlapped.GetLastErrorReturn;
			if (0 == pOverlapped.numberOfBytesTransferred)
			{
				LOG(WARNING) << ("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSARecv\n");
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			const auto [buf, len] = this->recvBuf.Complete(pOverlapped.numberOfBytesTransferred);
			if (len % 1000 == 0)
			{
				LOG(WARNING) << "����������" << len;
			}
			this->recvBuf.PopFront(this->Session.OnRecv(*this, buf, len));//�ص��û��Զ��庯���������Ǵ����ݣ���������û�У������WebSocketЭ�鸺��Ĺ���
		}
		{
			std::lock_guard lock(lockFinish);
			recvFinish = true;
			if (!sendFinish)
			{
				LOG(INFO) << "PostRecvЭ�̽���������sendOverlapped��û����,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return Overlapped::Error;
			}
		}
		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostRecvЭ�̽���,GetCurrentThreadId=" << GetCurrentThreadId();
		co_return Overlapped::OK;
	}
	template<class T_Session>
	CoTask<Overlapped::YieldReturn> SessionSocketCompletionKey<T_Session>::PostSend(Overlapped& overlapped)
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
			//if (overlapped.callSend)
			//	LOG(INFO) << "׼���첽�ȴ�WSASend���";
			//else
			//	LOG(INFO) << "���������ٷ�WSASend" ;

			//LOG(INFO) << "��ʼ�첽��WSASend���,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
				//<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len
				//<< ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn;
			co_yield overlapped.callSend ? Overlapped::Sending : Overlapped::SendStop;
			//LOG(INFO) << "���첽�ȵ�WSASend���,pOverlapped.numberOfBytesTransferred=" << overlapped.numberOfBytesTransferred
			//	<< ",callSend=" << overlapped.callSend << ",wsabuf.len=" << overlapped.wsabuf.len << ",GetLastErrorReturn=" << overlapped.GetLastErrorReturn
			//	<< ",Socket=" << Socket();

			if (!overlapped.callSend)
			{
				//LOG(INFO) << ("�������ˣ�׼����WSASend\n");
				continue;
			}


			if (0 == overlapped.numberOfBytesTransferred && overlapped.GetLastErrorReturn != ERROR_IO_PENDING)
			{
				LOG(WARNING) << ("numberOfBytesTransferred==0���ܶ�����,���ٵ���WSASend,pOverlapped.GetLastErrorReturn=%d,GetThreadId=%d\n", overlapped.GetLastErrorReturn, GetCurrentThreadId());
				CloseSocket();
				//delete pOverlapped;
				break;
			}
			//if (pOverlapped.GetLastErrorReturn == ERROR_IO_PENDING) 
			//{
			//	printf("ERROR_IO_PENDING��û���꣬�´λ�Ҫ���ŷ�\n");
			//}

			this->sendBuf.Complete(overlapped.numberOfBytesTransferred);
			overlapped.callSend = true;
		}

		//if (!this->recvOverlapped.coTask.Finished())

		{
			std::lock_guard lock(lockFinish);
			sendFinish = true;
			if (!recvFinish)
			{
				LOG(INFO) << "PostSendЭ�̽���������recvOverlapped��û����,GetCurrentThreadId=" << GetCurrentThreadId();
				co_return Overlapped::Error;
			}
		}

		//this->Session.OnDestroy();
		//delete this;
		LOG(INFO) << "PostSendЭ�̽���,GetCurrentThreadId=" << GetCurrentThreadId();
		co_return Overlapped::OK;
	}

	template<class T_Session>
	bool SessionSocketCompletionKey<T_Session>::WSARecv(Overlapped& refOverlapped)
	{

		DWORD dwRecvCount(0);
		DWORD dwFlag(0);
		refOverlapped.numberOfBytesTransferred = 0;
		std::tie(refOverlapped.wsabuf.buf, refOverlapped.wsabuf.len) = this->recvBuf.BuildRecvBuf();
		//refOverlapped.GetQueuedCompletionStatusReturn
		//LOG(INFO) << ("����::WSARecv\n");
		const auto recvRet = ::WSARecv(Socket(), &refOverlapped.wsabuf, 1, &dwRecvCount, &dwFlag, &refOverlapped.overlapped, NULL);
		//refOverlapped.GetLastErrorReturn 
		const auto err = WSAGetLastError();

		if (0 == recvRet)//���δ�����κδ��󣬲��ҽ��ղ�����������ɣ� �� WSARecv �����㡣 ����������£��ڵ����̴߳��ڿɾ���״̬�󣬽��Ѽƻ�����������̡�
		{
			return true;
		}

		if (SOCKET_ERROR != recvRet ||
			ERROR_IO_PENDING != err)
		{
			switch (err)
			{
			case WSAECONNABORTED:
				LOG(WARNING) << "An established connection was aborted by the software in your host machine.";
				break;
			case WSAENOTSOCK:
				LOG(WARNING) << "An operation was attempted on something that is not a socket.";
				break;

			}
			LOG(WARNING) << "WSARecv�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��err=" << err << ",Socket=" << Socket();
			return false;// �κ�����������붼ָʾ�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��
		}

		// ���򣬽����� ֵ SOCKET_ERROR �����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣 
		// ������� WSA_IO_PENDING ָʾ�ص������ѳɹ��������Ժ�ָʾ��ɡ� 
		return true;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="refOverlapped"></param>
	/// <param name="refSize"></param>
	/// <returns>�����ȴ��첽���,û������Ҫ����û�е���WSASend</returns>
	template<class T_Session>
	std::tuple<bool, bool>  SessionSocketCompletionKey<T_Session>::WSASend(Overlapped& refOverlapped)
	{
		DWORD dwFlag = 0;
		refOverlapped.numberOfBytesTransferred = 0;
		refOverlapped.dwSendCount = 0;
		std::tie(refOverlapped.wsabuf.buf, refOverlapped.wsabuf.len) = this->sendBuf.BuildSendBuf();
		if (nullptr == refOverlapped.wsabuf.buf)
		{
			return std::make_tuple(true, false);
		}
		//refOverlapped.GetQueuedCompletionStatusReturn
		int sendRet = ::WSASend(Socket(), &refOverlapped.wsabuf, 1, &refOverlapped.dwSendCount, dwFlag, &refOverlapped.overlapped, NULL);
		//refOverlapped.GetLastErrorReturn
		int err = WSAGetLastError();
		//refOverlapped.numberOfBytesTransferred = dwSendCount;
		if (0 == sendRet)
		{
			//LOG(INFO) << "WSASend�ص��Ĳ����ɹ��������Ѿ����,WSAGetLastError=" << err
			//	<< ",Socket=" << Socket() << ",wsabuf.len=" << refOverlapped.wsabuf.len << ",numberOfBytesTransferred=" << refOverlapped.numberOfBytesTransferred;
			return std::make_tuple(true, true);//���δ�����κδ��󣬲��ҷ��Ͳ�����������ɣ� �� WSASend �����㡣 ����������£�һ�������̴߳��ڿɾ���״̬�����Ѽƻ�����������̡�
		}
		if (SOCKET_ERROR != sendRet ||
			ERROR_IO_PENDING != err)
		{
			LOG(WARNING) << "WSASend�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��GetLastErrorReturn =" << refOverlapped.GetLastErrorReturn << ",err=" << err
				<< ",Socket=" << Socket() << ",wsabuf.len=" << refOverlapped.wsabuf.len;
			switch (err)
			{
			case WSAENOTSOCK:
				LOG(WARNING) << "An operation was attempted on something that is not a socket.";
				break;
			case WSAECONNABORTED:
				LOG(WARNING) << "An established connection was aborted by the software in your host machine.";
				break;
			}
			closesocket(Socket());
			return std::make_tuple(false, true);// �κ�����������붼ָʾ�ص��Ĳ���δ�ɹ����������Ҳ��ᷢ�����ָʾ��
		}

		// ���򣬽����� ֵ SOCKET_ERROR �����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣 
		// ������� WSA_IO_PENDING ָʾ�ص������ѳɹ��������Ժ�ָʾ��ɡ� 
		//LOG(INFO) << "WSASend�ص��Ĳ����ɹ�������WSAGetLastError=" << err;
		return std::make_tuple(true, true);
	}
}