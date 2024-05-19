#include "Client.h"

namespace Iocp
{
	bool Client::Connect(const wchar_t* szIp, const wchar_t* szPort, HANDLE hIocp)
	{
		auto socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		SOCKADDR_STORAGE LocalAddr = { 0 };
		SOCKADDR_STORAGE RemoteAddr = { 0 };
		DWORD dwLocalAddr = sizeof(LocalAddr);
		DWORD dwRemoteAddr = sizeof(RemoteAddr);
		const auto bSuccess = WSAConnectByName(socket, (LPWSTR)szIp, (LPWSTR)szPort, &dwLocalAddr,
			(SOCKADDR*)&LocalAddr,
			&dwRemoteAddr,
			(SOCKADDR*)&RemoteAddr,
			NULL,
			NULL);
		return bSuccess;
	}
}