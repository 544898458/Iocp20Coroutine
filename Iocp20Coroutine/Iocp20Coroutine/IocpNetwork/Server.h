#pragma once
#include <Winsock2.h>

#include<vector>


namespace Iocp 
{
	template<class T_Session>
	class Server
	{
	public:
		bool WsaStartup();
		bool Init();
		void Stop();
	private:
		static void NetworkThreadProc(LPVOID lpParameter);
		//bool flag = true;
	private:
		SOCKET socketAccept=NULL;
		HANDLE hIocp = NULL;
		std::vector<HANDLE> vecThread;
	};
}