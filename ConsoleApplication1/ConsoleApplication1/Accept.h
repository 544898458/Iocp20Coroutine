#pragma once
#include <Winsock2.h>

#include<vector>


namespace Iocp 
{
	
	class Accept
	{
	public:
		bool WsaStartup();
		bool Init();
	private:
		static DWORD WINAPI ThreadProc(LPVOID lpParameter);
		//bool flag = true;
	private:
		SOCKET socketAccept=NULL;
		HANDLE hIocp = nullptr;
		std::vector<HANDLE> vecThread;
	};

}