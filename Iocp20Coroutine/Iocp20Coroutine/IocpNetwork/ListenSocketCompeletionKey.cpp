#include <glog/logging.h>
#include "ListenSocketCompeletionKey.h"
#include "SessionSocketCompeletionKey.h"

namespace Iocp {
	template<class T_Session>
	void ListenSocketCompeletionKey<T_Session>::StartCoRoutine()
	{
		auto pAcceptOverlapped = new Overlapped();
		pAcceptOverlapped->coTask = PostAccept(pAcceptOverlapped);
		pAcceptOverlapped->coTask.Run();
	}
	template<class T_Session>
	std::tuple<bool, bool> ListenSocketCompeletionKey<T_Session>::AcceptEx(Overlapped* pAcceptOverlapped)
	{
		char str2[1];// = new char[1024];// = { 0 };
		DWORD dwRecvcount = 0;
		//int b = WSAGetLastError();
		//OVERLAPPED* Overlapped = new OVERLAPPED();
		pAcceptOverlapped->GetQueuedCompletionStatusReturn = ::AcceptEx(
			this->Socket(),	//[in]侦听套接字。服务器应用程序在这个套接字上等待连接。
			pAcceptOverlapped->socket,	//[in]将用于连接的套接字。此套接字必须不能已经绑定或者已经连接。
			str2, //[in]指向一个缓冲区，该缓冲区用于接收新建连接的所发送数据的第一个块、该服务器的本地地址和客户端的远程地址。接收到的数据将被写入到缓冲区0偏移处，而地址随后写入。 该参数必须指定，如果此参数设置为NULL，将不会得到执行，也无法通过GetAcceptExSockaddrs函数获得本地或远程的地址。
			0,	//[in]lpOutputBuffer字节数，指定接收数据缓冲区lpOutputBuffer的大小。这一大小应不包括服务器的本地地址的大小或客户端的远程地址，他们被追加到输出缓冲区。如果dwReceiveDataLength是零，AcceptEx将不等待接收任何数据，而是尽快建立连接。
			sizeof(struct sockaddr_in) + 16,//[in]为本地地址信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。
			sizeof(struct sockaddr_in) + 16,//[in]为远程地址的信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。 该值不能为0。
			&dwRecvcount,//[out]指向一个DWORD用于标识接收到的字节数。此参数只有在同步模式下有意义。如果函数返回ERROR_IO_PENDING并在迟些时候完成操作，那么这个DWORD没有意义，这时你必须获得从完成通知机制中读取操作字节数。
			&pAcceptOverlapped->overlapped//overlapped这里面内存必须清零，否则WSAGetLastError会返回ERROR_INVALID_HANDLE
		);
		pAcceptOverlapped->GetLastErrorReturn = WSAGetLastError();
		pAcceptOverlapped->numberOfBytesTransferred = dwRecvcount;
		if (pAcceptOverlapped->GetQueuedCompletionStatusReturn)//同步返回
		{
			assert(0 == pAcceptOverlapped->GetLastErrorReturn);
			return std::make_tuple(true, false);//AcceptEx(pAcceptOverlapped);
		}

		if (pAcceptOverlapped->GetLastErrorReturn == ERROR_IO_PENDING)
			return std::make_tuple(true, true);//异步通过IOCP返回

		//LOG(INFO) << "AcceptEx err="<< pAcceptOverlapped->GetLastErrorReturn;
		switch (pAcceptOverlapped->GetLastErrorReturn)
		{
		case ERROR_IO_INCOMPLETE:
			LOG(WARNING) << "AcceptEx失败 Overlapped I/O event is not in a signaled state.";
			break;
		case WSAENOTSOCK:
			LOG(WARNING) << "AcceptEx失败 An operation was attempted on something that is not a socket.";
			break;
		default:
			LOG(WARNING) << "AcceptEx失败 GetLastErrorReturn=" << pAcceptOverlapped->GetLastErrorReturn;
			break;
		}

		return std::make_tuple(false, false);
	}
	template<class T_Session>
	CoTask<int> ListenSocketCompeletionKey<T_Session>::PostAccept(Overlapped* pAcceptOverlapped)
	{
		while (true)
		{
			pAcceptOverlapped->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			bool acceptOk;
			bool async;
			std::tie(acceptOk, async) = AcceptEx(pAcceptOverlapped);
			if (!acceptOk)
			{
				LOG(WARNING) << "AcceptEx失败，停止Accept";
				co_return 0;
			}

			LOG(INFO) << "AcceptEx成功";

			if (async)
			{
				LOG(INFO) << "准备异步等待重叠AcceptEx完成\n";
				co_yield 0;
				LOG(INFO) << ("异步重叠AcceptEx完成\n");
			}

			if (!pAcceptOverlapped->GetQueuedCompletionStatusReturn)
			{
				switch (pAcceptOverlapped->GetLastErrorReturn)
				{
				case ERROR_OPERATION_ABORTED:
					LOG(WARNING) << ("The I/O operation has been aborted because of either a thread exit or an application request.");
					break;
				default:
					LOG(WARNING) << "AcceptEx失败,GetLastErrorReturn=" << pAcceptOverlapped->GetLastErrorReturn;
					break;
				}
				co_yield 0;
			}

			//绑定到完成端口
			auto pNewCompleteKey = new SessionSocketCompeletionKey<T_Session>(pAcceptOverlapped->socket);
			pNewCompleteKey->Session.OnInit(*pNewCompleteKey);//回调用户自定义函数
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)pAcceptOverlapped->socket, this->hIocp, (ULONG_PTR)pNewCompleteKey, 0);
			if (hPort1 != this->hIocp)
			{
				int a = GetLastError();
				LOG(INFO) << "连上来的Socket关联到完成端口失败，Error=" << a;
				//closesocket(pKey->socket);// all_socks[count]);
				delete pNewCompleteKey;
				co_return 0;
			}
			pNewCompleteKey->StartCoRoutine();
		}
	}
}