#include "pch.h"
#include <glog/logging.h>
#include "ListenSocketCompletionKey.h"

namespace Iocp
{
	std::tuple<bool, bool> ListenSocketCompletionKey::AcceptEx(Overlapped* pAcceptOverlapped, SOCKET socketListen)
	{
		pAcceptOverlapped->dwRecvcount = 0;
		//int b = WSAGetLastError();
		//OVERLAPPED* Overlapped = new OVERLAPPED();
		const auto b同步完成 = ::AcceptEx(
			socketListen,	//[in]侦听套接字。服务器应用程序在这个套接字上等待连接。
			pAcceptOverlapped->socket,	//[in]将用于连接的套接字。此套接字必须不能已经绑定或者已经连接。
			pAcceptOverlapped->bufOutput, //[in]指向一个缓冲区，该缓冲区用于接收新建连接的所发送数据的第一个块、该服务器的本地地址和客户端的远程地址。接收到的数据将被写入到缓冲区0偏移处，而地址随后写入。 该参数必须指定，如果此参数设置为NULL，将不会得到执行，也无法通过GetAcceptExSockaddrs函数获得本地或远程的地址。
			0,//sizeof(pAcceptOverlapped->bufOutput)-(sizeof(struct sockaddr_in) + 16)*2,	//[in]lpOutputBuffer字节数，指定接收数据缓冲区lpOutputBuffer的大小。这一大小应不包括服务器的本地地址的大小或客户端的远程地址，他们被追加到输出缓冲区。如果dwReceiveDataLength是零，AcceptEx将不等待接收任何数据，而是尽快建立连接。
			sizeof(struct sockaddr_in) + 16,//[in]为本地地址信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。
			sizeof(struct sockaddr_in) + 16,//[in]为远程地址的信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。 该值不能为0。
			&pAcceptOverlapped->dwRecvcount,//[out]指向一个DWORD用于标识接收到的字节数。此参数只有在同步模式下有意义。如果函数返回ERROR_IO_PENDING并在迟些时候完成操作，那么这个DWORD没有意义，这时你必须获得从完成通知机制中读取操作字节数。
			&pAcceptOverlapped->overlapped//overlapped这里面内存必须清零，否则WSAGetLastError会返回ERROR_INVALID_HANDLE
		);

		const auto lastError = WSAGetLastError();
		LOG(INFO) << "AcceptEx GetLastErrorReturn=" << pAcceptOverlapped->GetLastErrorReturn << ",dwRecvcount=" << pAcceptOverlapped->dwRecvcount
			<< ",b同步完成=" << b同步完成 << ",lastError=" << lastError;
		pAcceptOverlapped->GetLastErrorReturn = lastError;
		pAcceptOverlapped->GetQueuedCompletionStatusReturn = b同步完成;
		pAcceptOverlapped->numberOfBytesTransferred = pAcceptOverlapped->dwRecvcount;
		//返回值
		//	如果没有错误发生，那么AcceptEx在成功完成后返回true。
		//	如果函数失败，那么会返回false，使用WSAGetLastError函数获取错误信息，如果WSAGetLastError函数返回ERROR_IO_PENDING
		//	那么投递的异步操作将会在稍后完成，如果WSAGetLastError返回的错误是WSAECONNRESET，说明客户端的连接被关闭了。

		if (b同步完成)//同步返回
		{
			LOG(INFO) << "密集Accept导致同步完成" << pAcceptOverlapped->GetLastErrorReturn;
			_ASSERT0 == pAcceptOverlapped->GetLastErrorReturn);
			return std::make_tuple(true, false);//AcceptEx(pAcceptOverlapped);
		}

		if (pAcceptOverlapped->GetLastErrorReturn == ERROR_IO_PENDING)
			return std::make_tuple(true, true);//异步通过IOCP返回

		LOG(INFO) << "AcceptEx err=" << pAcceptOverlapped->GetLastErrorReturn;
		switch (pAcceptOverlapped->GetLastErrorReturn)
		{
		case ERROR_IO_INCOMPLETE:
			LOG(WARNING) << "AcceptEx失败 Overlapped I/O event is not in a signaled state.";
			break;
		case WSAENOTSOCK:
			LOG(WARNING) << "AcceptEx失败 An operation was attempted on something that is not a socket.";
			break;
		case ERROR_NETNAME_DELETED:
			LOG(WARNING) << "AcceptEx失败 The specified network name is no longer available.";
			break;
		default:
			LOG(WARNING) << "AcceptEx失败 GetLastErrorReturn=" << pAcceptOverlapped->GetLastErrorReturn;
			break;
		}

		return std::make_tuple(false, false);
	}
	//std::tuple<bool, bool> ListenSocketCompletionKey::AcceptEx(Overlapped* pAcceptOverlapped, SOCKET socketListen)
	//{
	//	char str2[1];// = new char[1024];// = { 0 };
	//	DWORD dwRecvcount = 0;
	//	//int b = WSAGetLastError();
	//	//OVERLAPPED* Overlapped = new OVERLAPPED();
	//	pAcceptOverlapped->GetQueuedCompletionStatusReturn = ::AcceptEx(
	//		socketListen,	//[in]侦听套接字。服务器应用程序在这个套接字上等待连接。
	//		pAcceptOverlapped->socket,	//[in]将用于连接的套接字。此套接字必须不能已经绑定或者已经连接。
	//		str2, //[in]指向一个缓冲区，该缓冲区用于接收新建连接的所发送数据的第一个块、该服务器的本地地址和客户端的远程地址。接收到的数据将被写入到缓冲区0偏移处，而地址随后写入。 该参数必须指定，如果此参数设置为NULL，将不会得到执行，也无法通过GetAcceptExSockaddrs函数获得本地或远程的地址。
	//		0,	//[in]lpOutputBuffer字节数，指定接收数据缓冲区lpOutputBuffer的大小。这一大小应不包括服务器的本地地址的大小或客户端的远程地址，他们被追加到输出缓冲区。如果dwReceiveDataLength是零，AcceptEx将不等待接收任何数据，而是尽快建立连接。
	//		sizeof(struct sockaddr_in) + 16,//[in]为本地地址信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。
	//		sizeof(struct sockaddr_in) + 16,//[in]为远程地址的信息保留的字节数。此值必须比所用传输协议的最大地址大小长16个字节。 该值不能为0。
	//		&dwRecvcount,//[out]指向一个DWORD用于标识接收到的字节数。此参数只有在同步模式下有意义。如果函数返回ERROR_IO_PENDING并在迟些时候完成操作，那么这个DWORD没有意义，这时你必须获得从完成通知机制中读取操作字节数。
	//		&pAcceptOverlapped->overlapped//overlapped这里面内存必须清零，否则WSAGetLastError会返回ERROR_INVALID_HANDLE
	//	);
	//	pAcceptOverlapped->GetLastErrorReturn = WSAGetLastError();
	//	pAcceptOverlapped->numberOfBytesTransferred = dwRecvcount;
	//	if (pAcceptOverlapped->GetQueuedCompletionStatusReturn)//同步返回
	//	{
	//		_ASSERT0 == pAcceptOverlapped->GetLastErrorReturn);
	//		return std::make_tuple(true, false);//AcceptEx(pAcceptOverlapped);
	//	}

	//	if (pAcceptOverlapped->GetLastErrorReturn == ERROR_IO_PENDING)
	//		return std::make_tuple(true, true);//异步通过IOCP返回

	//	//LOG(INFO) << "AcceptEx err="<< pAcceptOverlapped->GetLastErrorReturn;
	//	switch (pAcceptOverlapped->GetLastErrorReturn)
	//	{
	//	case ERROR_IO_INCOMPLETE:
	//		LOG(WARNING) << "AcceptEx失败 Overlapped I/O event is not in a signaled state.";
	//		break;
	//	case WSAENOTSOCK:
	//		LOG(WARNING) << "AcceptEx失败 An operation was attempted on something that is not a socket.";
	//		break;
	//	default:
	//		LOG(WARNING) << "AcceptEx失败 GetLastErrorReturn=" << pAcceptOverlapped->GetLastErrorReturn;
	//		break;
	//	}

	//	return std::make_tuple(false, false);
	//}
}