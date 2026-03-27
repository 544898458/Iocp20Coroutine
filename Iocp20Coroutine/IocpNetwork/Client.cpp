#include "pch.h"
#include "Client.h"
#include <assert.h>
#include <sstream>
namespace Iocp
{
	SOCKET Client::Connect(const char* szIp, const uint32_t wPort)
	{
		std::wstringstream ossPort,ossIp;
		ossIp << szIp;
		ossPort << wPort;
		const auto socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		SOCKADDR_STORAGE LocalAddr = { 0 };
		SOCKADDR_STORAGE RemoteAddr = { 0 };
		DWORD dwLocalAddr = sizeof(LocalAddr);
		DWORD dwRemoteAddr = sizeof(RemoteAddr);
		const auto bSuccess = WSAConnectByNameW(socket, ossIp.str().data(), ossPort.str().data(), &dwLocalAddr,
			(SOCKADDR*)&LocalAddr,
			&dwRemoteAddr,
			(SOCKADDR*)&RemoteAddr,
			NULL,
			NULL);
		_ASSERT(bSuccess);
		return socket; 
	}
}