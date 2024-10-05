#include "pch.h"
#include "Client.h"
#include <assert.h>
#include <sstream>
namespace Iocp
{
	SOCKET Client::Connect(const wchar_t* szIp, const uint32_t wPort)
	{
		std::wostringstream woss;
		woss << wPort;
		const auto socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		SOCKADDR_STORAGE LocalAddr = { 0 };
		SOCKADDR_STORAGE RemoteAddr = { 0 };
		DWORD dwLocalAddr = sizeof(LocalAddr);
		DWORD dwRemoteAddr = sizeof(RemoteAddr);
		const auto bSuccess = WSAConnectByName(socket, (LPWSTR)szIp, woss.str().data(), &dwLocalAddr,
			(SOCKADDR*)&LocalAddr,
			&dwRemoteAddr,
			(SOCKADDR*)&RemoteAddr,
			NULL,
			NULL);
		assert(bSuccess);
		return socket; 
	}
}