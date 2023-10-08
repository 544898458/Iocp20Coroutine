#pragma once
namespace Iocp 
{
	class Accept
	{
	public:
		bool WsaStartup();
		bool Init();
	private:
		SOCKET socketServer=NULL;
		HANDLE hIocp = nullptr;
		HANDLE hIocpAccept = nullptr;
	};

}