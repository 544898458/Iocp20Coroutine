#include "pch.h"
#include "ThreadPool.h"
#include "SocketCompeletionKey.h"
#include <set>
namespace Iocp::ThreadPool
{
	//std::set<HANDLE> g_setHandle;

	static bool NetworkThreadProcOneIocp(HANDLE port)
	{
		DWORD      number_of_bytes = 0;
		SocketCompeletionKey* pCompletionKey = nullptr;
		LPOVERLAPPED lpOverlapped;

		//pCompletionKey��Ӧһ��socket��lpOverlapped��Ӧһ���¼�

		//LOG(INFO) << "��GetQueuedCompletionStatus����,port=" << port << ",GetCurrentThreadId=" << GetCurrentThreadId();
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&pCompletionKey, &lpOverlapped, INFINITE);//û��ɾͻῨ���������
		int lastErr = GetLastError();//������Socketǿ�ƹر�
		//LOG(INFO) << "GetQueuedCompletionStatus����lastErr=" << lastErr << ",pCompletionKey=" << pCompletionKey << ",number_of_bytes=" << number_of_bytes;

		if (lpOverlapped != nullptr)
		{
			auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
			(overlapped->*overlapped->OnComplete)(pCompletionKey, port, number_of_bytes, bFlag, lastErr);
			if (overlapped->needDeleteMe && overlapped->coTask.Finished())
			{
				LOG(INFO) << "ɾ��" << overlapped->coTask.m_desc;
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
	static HANDLE m_hIocp = nullptr;

	static void NetworkThreadProc()
	{
		while (true) //��û���˳�����
		{
			//for (auto iter = g_setHandle.begin(); iter != g_setHandle.end(); ++iter)
			{
				if (NetworkThreadProcOneIocp(m_hIocp))
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
	
	void ThreadPool::Init()
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
			return ;
		}

		//�����߳���������
		SYSTEM_INFO system_processors_count;
		GetSystemInfo(&system_processors_count);
		auto process_count = 1;//���߳�������,send���� system_processors_count.dwNumberOfProcessors;
		for (decltype(process_count) i = 0; i < process_count; i++)
		{
			//auto hThread = CreateThread(NULL, 0, NetworkThreadProc, pListenCompleteKey->hIocp, 0, NULL);
			std::thread networkThread(NetworkThreadProc);
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
	const HANDLE& GetIocp(){ return m_hIocp; }
}