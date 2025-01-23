#include "pch.h"
#include "ThreadPool.h"
#include "SocketCompeletionKey.h"
#include <set>
namespace Iocp
{
	//std::set<HANDLE> g_setHandle;

	bool ThreadPool::NetworkThreadProcOneIocp()
	{
		DWORD      number_of_bytes = 0;
		SocketCompeletionKey* pCompletionKey = nullptr;
		LPOVERLAPPED lpOverlapped;

		//pCompletionKey对应一个socket，lpOverlapped对应一次事件

		//LOG(INFO) << "等GetQueuedCompletionStatus返回,port=" << port << ",GetCurrentThreadId=" << GetCurrentThreadId();
		BOOL bFlag = GetQueuedCompletionStatus(m_hIocp, &number_of_bytes, (PULONG_PTR)&pCompletionKey, &lpOverlapped, INFINITE);//没完成就会卡在这里，正常
		int lastErr = GetLastError();//可能是Socket强制关闭
		//LOG(INFO) << "GetQueuedCompletionStatus返回lastErr=" << lastErr << ",pCompletionKey=" << pCompletionKey << ",number_of_bytes=" << number_of_bytes;

		if (lpOverlapped != nullptr)
		{
			auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
			(overlapped->*overlapped->OnComplete)(pCompletionKey, m_hIocp, number_of_bytes, bFlag, lastErr);
			if (overlapped->needDeleteMe && overlapped->coTask.Finished())
			{
				LOG(INFO) << "删除:	" << overlapped->coTask.m_desc << ",pOverlapped=" << overlapped->pOverlapped;
				if (overlapped->pOverlapped)
				{
					assert(overlapped->pOverlapped->pOverlapped == overlapped);
					overlapped->pOverlapped->pOverlapped = nullptr;
					overlapped->pOverlapped = nullptr;
				}
				delete overlapped;
				overlapped = nullptr;
			}
		}
		else
		{
			LOG(INFO) << "GetQueuedCompletionStatus,lpOverlapped is null";
		}

		if (!bFlag)
		{
			switch (lastErr)
			{
			case ERROR_OPERATION_ABORTED:
				LOG(WARNING) << "The I/O operation has been aborted because of either a thread exit or an application request.";
				break;
			}
			return false;
		}

		return true;
	}

	void ThreadPool::NetworkThreadProc(ThreadPool& refThis)
	{
		while (true) //还没做退出功能
		{
			//for (auto iter = g_setHandle.begin(); iter != g_setHandle.end(); ++iter)
			{
				if (refThis.NetworkThreadProcOneIocp())
					continue;

				//iter = g_setHandle.erase(iter);
			}
		}
	}
	//bool Add(HANDLE hIocp)
	//{
	//	const auto ret = g_setHandle.insert(hIocp);
	//	LOG(INFO) << "Add,hIocp=" << hIocp << ",ret=" << ret.second << ",GetCurrentThreadId=" << GetCurrentThreadId();
	//	return ret.second;
	//}

	void ThreadPool::Init(const uint16_t usThreadCount)
	{
		//创建完成端口	创建一个I/O完成端口对象，用它面向任意数量的套接字句柄，管理多个I/O请求。要做到这一点，需要调用CreateCompletionPort函数。
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (0 == m_hIocp)
		{
			int a = GetLastError();
			LOG(INFO) << a;
			//closesocket(m_socketAccept);
			//清理网络库
			//WSACleanup();
			return;
		}

		//创建线程数量有了
		//SYSTEM_INFO system_processors_count;
		//GetSystemInfo(&system_processors_count);
		//多线程有问题,send那里 system_processors_count.dwNumberOfProcessors;
		for (int i = 0; i < usThreadCount; i++)
		{
			//auto hThread = CreateThread(NULL, 0, NetworkThreadProc, pListenCompleteKey->hIocp, 0, NULL);
			std::thread networkThread(&ThreadPool::NetworkThreadProc, std::ref(*this));
			networkThread.detach();
			//if (NULL == hThread)
			//{
			//	int a = GetLastError();
			//	printf("%d\n", a);
			//	CloseHandle(iocp);
			//	closesocket(socketAccept);
			//	//清理网络库
			//	//WSACleanup();
			//	return false;
			//}
			//this->vecThread.push_back(hThread);
		}
	}
	const HANDLE& ThreadPool::GetIocp() const { return m_hIocp; }
}