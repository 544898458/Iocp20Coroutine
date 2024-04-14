#include <glog/logging.h>
#include "ListenSocketCompeletionKey.h"
#include "SessionSocketCompeletionKey.h"


namespace Iocp {

	template<class T_Session,class T_Server>
		requires requires(Iocp::SessionSocketCompeletionKey<T_Session>& refCompletetionKeySession, T_Session& refSession, T_Server& refServer)
	{
		requires std::is_same_v<void, decltype(refSession.OnInit(refCompletetionKeySession, refServer))>;//void OnInit(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>& refSession, T_Server&);
	}
	void ListenSocketCompeletionKey::StartCoRoutine( HANDLE hIocp,SOCKET socketListen, T_Server &refServer)
	{
		auto pAcceptOverlapped = new Overlapped();
		pAcceptOverlapped->needDeleteMe = true;
		pAcceptOverlapped->coTask = PostAccept<T_Session>(pAcceptOverlapped,hIocp, socketListen, refServer);
		pAcceptOverlapped->coTask.m_desc = "PostAccept";
		pAcceptOverlapped->coTask.Run();
	}
	
	template<class T_Session, class T_Server>
	//	requires requires(T_Session& refSession, T_Server& refServer)
	//{
	//	requires std::is_same_v<void, decltype(refSession.OnInit(&refSession, refServer))>;//void MySession::OnInit(WebSocketSession<MySession>* pWsSession, MyServer& server)
	//}
	CoTask<int> ListenSocketCompeletionKey::PostAccept(Overlapped* pAcceptOverlapped,HANDLE hIocp, SOCKET socketListen, T_Server& refServer)
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
				co_return 0;
			}

			LOG(INFO) << "AcceptEx�ɹ�";

			if (async)
			{
				LOG(INFO) << "׼���첽�ȴ��ص�AcceptEx���\n";
				co_yield 0;
				LOG(INFO) << ("�첽�ص�AcceptEx���\n");
			}

			if (!pAcceptOverlapped->GetQueuedCompletionStatusReturn)
			{
				switch (pAcceptOverlapped->GetLastErrorReturn)
				{
				case ERROR_OPERATION_ABORTED:
					LOG(WARNING) << ("The I/O operation has been aborted because of either a thread exit or an application request.");
					break;
				default:
					LOG(WARNING) << "AcceptExʧ��,GetLastErrorReturn=" << pAcceptOverlapped->GetLastErrorReturn;
					break;
				}
				closesocket(pAcceptOverlapped->socket);
				pAcceptOverlapped->socket = NULL;
				co_return 0;
			}

			//�󶨵���ɶ˿�
			auto pNewCompleteKey = new SessionSocketCompeletionKey<T_Session>(pAcceptOverlapped->socket);
			pNewCompleteKey->Session.OnInit(*pNewCompleteKey,refServer);//�ص��û��Զ��庯��
			refServer.OnAdd(*pNewCompleteKey);
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)pAcceptOverlapped->socket, hIocp, (ULONG_PTR)pNewCompleteKey, 0);
			if (hPort1 != hIocp)
			{
				int a = GetLastError();
				LOG(INFO) << "��������Socket��������ɶ˿�ʧ�ܣ�Error=" << a;
				//closesocket(pKey->socket);// all_socks[count]);
				delete pNewCompleteKey;
				co_return 0;
			}
			pNewCompleteKey->StartCoRoutine();
		}
	}
}