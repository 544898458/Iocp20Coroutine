#pragma once
#include <Winsock2.h>
namespace Iocp::ThreadPool
{
	void Init();
	bool Add(HANDLE hIocp);
	const HANDLE& GetIocp();
}