#pragma once
#include <Winsock2.h>
#include<type_traits>
#include"SessionSocketCompletionKey.h"
namespace Iocp
{
	/// <summary>
	/// Server����ҪT_Session���������ﲻ����T_Session��Ա����
	/// T_Sessionֻ��Init��ʱ����Ҫ����
	/// </summary>
	/// <typeparam name="T_Server"></typeparam>
	template<class T_Server>
	class Server
	{
	public:
		Server(const HANDLE& hIocp) :m_hIocp(hIocp)
		{

		}
		template<class T_Session>
			requires requires(Iocp::SessionSocketCompletionKey<T_Session>& refCompletionKeySession, T_Session& refSession, T_Server& refServer)
		{
			requires std::is_same_v<int, decltype(refSession.OnRecv(refCompletionKeySession, (const void*)nullptr, 0))>;//int OnRecv(Iocp::SessionSocketCompletionKey<WorldSession>& refSession, const char buf[], int len)
			requires std::is_same_v<void, decltype(refSession.OnDestroy())>;//void OnDestroy();
			requires std::is_same_v<void, decltype(refServer.OnAdd(refCompletionKeySession))>;//void OnAdd(Iocp::SessionSocketCompletionKey<WorldSession>& session)
		}
		bool Init(const uint16_t usPort);
		void Stop();
		

		T_Server m_Server;
	private:
		SOCKET m_socketAccept = NULL;
		const HANDLE& m_hIocp;
	};
}