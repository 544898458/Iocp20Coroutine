#pragma once
#include <Winsock2.h>
namespace Iocp
{
	class ThreadPool
	{
	public:
		void Init(const uint16_t usThreadCount=5);
		const HANDLE& GetIocp()const;
		static bool Is在主线程();
	private:
		static void NetworkThreadProc(ThreadPool& refThis);
		bool NetworkThreadProcOneIocp();
		HANDLE m_hIocp = nullptr;
		static std::thread::id m_id主线程;
	};
}