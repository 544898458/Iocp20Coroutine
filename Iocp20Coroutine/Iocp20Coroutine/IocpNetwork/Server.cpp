#include <glog/logging.h>

#include <Winsock2.h>

#include <WS2tcpip.h>
#include <mswsock.h>
#include <winnt.h>
#include "Server.h"
#include "./ListenSocketCompeletionKey.h"
#pragma comment(lib,"ws2_32.lib")

//template<class T_Session>
bool Iocp::Server::WsaStartup()
{
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdScokMsg;
	int nRes = WSAStartup(wdVersion, &wdScokMsg);

	if (0 == nRes)
		return true;
	{
		switch (nRes)
		{
		case WSASYSNOTREADY:
			LOG(INFO) << "重启下电脑试试，或者检查网络库";
			break;
		case WSAVERNOTSUPPORTED:
			LOG(INFO) << ("请更新网络库");
			break;
		case WSAEINPROGRESS:
			LOG(INFO) << ("请重新启动");
			break;
		case WSAEPROCLIM:
			LOG(INFO) << ("请尝试关掉不必要的软件，以为当前网络运行提供充足资源");
			break;
		}

		return false;
	}
}

//template<class T_Session>
void Iocp::Server::Stop()
{
	closesocket(this->m_socketAccept);
	this->m_socketAccept = NULL;
	CloseHandle(this->m_hIocp);
}

void Iocp::Server::NetworkThreadProc(HANDLE port)
{
	DWORD      number_of_bytes = 0;
	SocketCompeletionKey* pCompletionKey = nullptr;
	LPOVERLAPPED lpOverlapped;
	while (true)
	{
		//pCompletionKey对应一个socket，lpOverlapped对应一次事件
		BOOL bFlag = GetQueuedCompletionStatus(port, &number_of_bytes, (PULONG_PTR)&pCompletionKey, &lpOverlapped, INFINITE);//没完成就会卡在这里，正常
		int lastErr = GetLastError();//可能是Socket强制关闭
		if (lpOverlapped != nullptr)
		{
			auto* overlapped = (Iocp::Overlapped*)lpOverlapped;
			overlapped->OnComplete(pCompletionKey, port, number_of_bytes, bFlag, lastErr);
			if (overlapped->needDeleteMe && overlapped->coTask.Finished())
			{
				LOG(INFO) << "删除" << overlapped->coTask.m_desc;
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
			return;
		}
	}
}

