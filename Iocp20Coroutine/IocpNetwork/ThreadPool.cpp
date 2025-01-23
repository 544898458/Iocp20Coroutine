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

		//pCompletionKey��Ӧһ��socket��lpOverlapped��Ӧһ���¼�

		//LOG(INFO) << "��GetQueuedCompletionStatus����,port=" << port << ",GetCurrentThreadId=" << GetCurrentThreadId();
		BOOL bFlag = GetQueuedCompletionStatus(m_hIocp, &number_of_bytes, (PULONG_PTR)&pCompletionKey, &lpOverlapped, INFINITE);//û��ɾͻῨ���������
		int lastErr = GetLastError();//������Socketǿ�ƹر�
		//LOG(INFO) << "GetQueuedCompletionStatus����lastErr=" << lastErr << ",pCompletionKey=" << pCompletionKey << ",number_of_bytes=" << number_of_bytes;

		if (lpOverlapped != nullptr)
		{
			auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
			(overlapped->*overlapped->OnComplete)(pCompletionKey, m_hIocp, number_of_bytes, bFlag, lastErr);
			if (overlapped->needDeleteMe && overlapped->coTask.Finished())
			{
				LOG(INFO) << "ɾ��:	" << overlapped->coTask.m_desc << ",pOverlapped=" << overlapped->pOverlapped;
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
		while (true) //��û���˳�����
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
		//������ɶ˿�	����һ��I/O��ɶ˿ڶ����������������������׽��־����������I/O����Ҫ������һ�㣬��Ҫ����CreateCompletionPort������
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (0 == m_hIocp)
		{
			int a = GetLastError();
			LOG(INFO) << a;
			//closesocket(m_socketAccept);
			//���������
			//WSACleanup();
			return;
		}

		//�����߳���������
		//SYSTEM_INFO system_processors_count;
		//GetSystemInfo(&system_processors_count);
		//���߳�������,send���� system_processors_count.dwNumberOfProcessors;
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
			//	//���������
			//	//WSACleanup();
			//	return false;
			//}
			//this->vecThread.push_back(hThread);
		}
	}
	const HANDLE& ThreadPool::GetIocp() const { return m_hIocp; }
}