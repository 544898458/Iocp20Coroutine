#include <glog/logging.h>
#include "ListenSocketCompletionKey.h"
#include "SessionSocketCompletionKey.h"


namespace Iocp {

	template<class T_Session, class T_Server>
		requires requires(Iocp::SessionSocketCompletionKey<T_Session>& refCompletionKeySession, T_Session& refSession, T_Server& refServer)
	{
		requires std::is_same_v<void, decltype(refSession.OnInit(refServer))>;//void OnInit(Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>& refSession, T_Server&);
	}
	void ListenSocketCompletionKey::StartCoRoutine(HANDLE hIocp, SOCKET socketListen, T_Server& refServer)
	{
		auto pAcceptOverlapped = new Overlapped();
		pAcceptOverlapped->needDeleteMe = true;
		pAcceptOverlapped->coTask = PostAccept<T_Session>(pAcceptOverlapped, hIocp, socketListen, refServer);
		pAcceptOverlapped->coTask.m_desc = "PostAccept";
		pAcceptOverlapped->coTask.Run();
	}
	
	template<class T_Session, class T_Server>
	//	requires requires(T_Session& refSession, T_Server& refServer)
	//{
	//	requires std::is_same_v<void, decltype(refSession.OnInit(&refSession, refServer))>;//void MySession::OnInit(WebSocketSession<MySession>* pWsSession, MyServer& server)
	//}
	CoTask<Overlapped::YieldReturn> ListenSocketCompletionKey::PostAccept(Overlapped* pAcceptOverlapped, HANDLE hIocp, SOCKET socketListen, T_Server& refServer)
	{
		while (true)
		{
			pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			bool acceptOk;
			bool async;
			std::tie(acceptOk, async) = AcceptEx(pAcceptOverlapped, socketListen);
			if (!acceptOk)
			{
				LOG(WARNING) << "AcceptExʧ�ܣ�ֹͣAccept";
				closesocket(pAcceptOverlapped->socket);
				pAcceptOverlapped->socket = NULL;
				//co_return Overlapped::Error;
				continue;//20241226
			}

			LOG(INFO) << "AcceptEx�ɹ�,Socket=" << pAcceptOverlapped->socket;

			if (async)
			{
				LOG(INFO) << "׼���첽�ȴ��ص�AcceptEx���,socket=" << pAcceptOverlapped->socket;
				co_yield Overlapped::OK;
				LOG(INFO) << "�첽�ص�AcceptEx���,socket=" << pAcceptOverlapped->socket << ",async=" << async;
			}

			if (!pAcceptOverlapped->GetQueuedCompletionStatusReturn)
			{
				switch (pAcceptOverlapped->GetLastErrorReturn)
				{
				case ERROR_OPERATION_ABORTED:
					LOG(ERROR) << ("The I/O operation has been aborted because of either a thread exit or an application request.");
					break;
				case ERROR_NETNAME_DELETED:
					LOG(ERROR) << "The specified network name is no longer available.";
					break;
				case ERROR_IO_PENDING:
					LOG(ERROR) << "Overlapped I/O operation is in progress.";
					break;
				default:
					LOG(ERROR) << "AcceptExʧ��,GetLastErrorReturn=" << pAcceptOverlapped->GetLastErrorReturn;
					//_ASSERTfalse);
					break;
				}
				closesocket(pAcceptOverlapped->socket);
				pAcceptOverlapped->socket = NULL;
				//co_return Overlapped::Error;
				continue;
			}

			//�󶨵���ɶ˿�
			auto pNewCompleteKey = new SessionSocketCompletionKey<T_Session>(pAcceptOverlapped->socket);
			pNewCompleteKey->Session.OnInit(refServer);//�ص��û��Զ��庯��
			refServer.OnAdd(*pNewCompleteKey);
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)pAcceptOverlapped->socket, hIocp, (ULONG_PTR)pNewCompleteKey, 0);
			if (hPort1 != hIocp)
			{
				const int err = GetLastError();
				switch (err)
				{
				case ERROR_INVALID_PARAMETER:
					LOG(ERROR) << "The parameter is incorrect.";
					break;
				default:
					LOG(ERROR) << "AcceptEx���CreateIoCompletionPortʧ��,err=" << err;
					//_ASSERTfalse);
					break;
				}
				LOG(ERROR) << "��������Socket��������ɶ˿�ʧ�ܣ�Error=" << err << ",socket:"<< pAcceptOverlapped->socket;
				//closesocket(pKey->socket);// all_socks[count]);
				delete pNewCompleteKey;
				//co_return Overlapped::OK;
				continue;
			}
			pNewCompleteKey->StartCoRoutine(hIocp);
		}


		LOG(INFO) << "������ִ�е�����";
		co_return Overlapped::OK;
	}
}