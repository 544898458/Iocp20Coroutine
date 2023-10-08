#pragma once
#include<vector>
namespace Iocp 
{
	struct TcpSession
	{
		SOCKET socket;
		OVERLAPPED overlapped;
	};
	class Accept
	{
	public:
		bool WsaStartup();
		bool Init();
	private:
		int PostAccept();
	private:
		SOCKET socketAccept=NULL;
		HANDLE hIocpAccept = nullptr;
		std::vector<HANDLE> vecThread;
	};

}