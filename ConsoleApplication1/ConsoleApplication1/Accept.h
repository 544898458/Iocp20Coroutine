#pragma once
#include<vector>


namespace Iocp 
{
	
	class Accept
	{
	public:
		bool WsaStartup();
		bool Init();
	private:
		bool PostAccept();
		static DWORD WINAPI ThreadProc(LPVOID lpParameter);
		//bool flag = true;
	private:
		SOCKET socketAccept=NULL;
		HANDLE hIocp = nullptr;
		std::vector<HANDLE> vecThread;
	};

}