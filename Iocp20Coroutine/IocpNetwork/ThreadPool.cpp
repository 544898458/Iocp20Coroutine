#include "ThreadPool.h"
#include "SocketCompeletionKey.h"
#include <set>
namespace Iocp
{
	std::set<HANDLE> g_setHandle;

	static bool NetworkThreadProcOneIocp(HANDLE port)
	{
		DWORD      number_of_bytes = 0;
		SocketCompeletionKey* pCompletionKey = nullptr;
		LPOVERLAPPED lpOverlapped;

		//pCompletionKey��Ӧһ��socket��lpOverlapped��Ӧһ���¼�
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&pCompletionKey, &lpOverlapped, INFINITE);//û��ɾͻῨ���������
		int lastErr = GetLastError();//������Socketǿ�ƹر�
		if (lpOverlapped != nullptr)
		{
			auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
			overlapped->OnComplete(pCompletionKey, port, number_of_bytes, bFlag, lastErr);
			if (overlapped->needDeleteMe && overlapped->coTask.Finished())
			{
				LOG(INFO) << "ɾ��" << overlapped->coTask.m_desc;
				delete overlapped;
				overlapped = nullptr;
			}
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

	static void NetworkThreadProc()
	{
		for (auto iter = g_setHandle.begin(); iter != g_setHandle.end(); ++iter)
		{
			if (NetworkThreadProcOneIocp(*iter))
				continue;

			iter = g_setHandle.erase(iter);
		}
	}
	bool Add(HANDLE hIocp)
	{
		const auto ret = g_setHandle.insert(hIocp);
		return ret.second;
	}
	void ThreadPool::Init()
	{
		//�����߳���������
		SYSTEM_INFO system_processors_count;
		GetSystemInfo(&system_processors_count);
		auto process_count = system_processors_count.dwNumberOfProcessors;
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
}