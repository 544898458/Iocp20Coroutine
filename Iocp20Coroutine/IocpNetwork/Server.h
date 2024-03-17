#pragma once
#include <Winsock2.h>

#include<vector>
#include<type_traits>

namespace Iocp 
{
	template<class T_Server>
	class Server
	{
	public:
		bool WsaStartup();
		template<class T_Session>
			requires requires(T_Session &refSession)
		{
			//std::is_function_v<decltype(T_Session::OnInit)>;
			std::is_function_v<decltype(T_Session::OnRecv)>;
			std::is_function_v<decltype(T_Session::OnDestroy)>;
		}
		bool Init(const uint16_t usPort);
		void Stop();
		void Connect(const wchar_t *szIp, const wchar_t* szPort);
		T_Server m_Server;
	private:
		static void NetworkThreadProc(HANDLE port);
	private:
		SOCKET m_socketAccept=NULL;
		HANDLE m_hIocp = NULL;
		//std::vector<HANDLE> vecThread;
		
	};
}