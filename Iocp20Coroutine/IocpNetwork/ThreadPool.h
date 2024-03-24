#pragma once
#include <Winsock2.h>
namespace Iocp 
{
	namespace ThreadPool
	{
		static void Init();
		static void Add(HANDLE hIocp);
	};

}