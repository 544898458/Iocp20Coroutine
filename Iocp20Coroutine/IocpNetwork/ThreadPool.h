#pragma once
#include <Winsock2.h>
namespace Iocp
{
	class ThreadPool
	{
	public:
		void Init(const uint16_t usThreadCount=5);
		const HANDLE& GetIocp()const;
	private:
		static void NetworkThreadProc(ThreadPool& refThis);
		bool NetworkThreadProcOneIocp();
		HANDLE m_hIocp = nullptr;
	};
}