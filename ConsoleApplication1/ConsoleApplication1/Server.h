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
	private:
		static DWORD WINAPI ThreadProc(LPVOID lpParameter);
		//bool flag = true;
	private:
		SOCKET socketAccept=NULL;
		std::vector<HANDLE> vecThread;
	};
}