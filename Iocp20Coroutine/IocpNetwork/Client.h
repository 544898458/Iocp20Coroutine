#pragma once

#include <WinSock2.h>
#include <stdint.h>

namespace Iocp
{
	class Client
	{
	public:
		bool Connect(const wchar_t* szIp, const wchar_t* szPort, HANDLE hIocp);
	};
}
