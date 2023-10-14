#include "SessionSocketCompeletionKey.h"

//virtual bool PostAccept(MyOverlapped* pAcceptOverlapped)override 
//{
//	assert(!"SessionSocketCompeletionKey不能PostAccept");
//	return false;
//}

inline void SessionSocketCompeletionKey::StartCoRoutine()
{
	{
		//auto pOverlapped = new MyOverlapped();
		//PostSend(pOverlapped);
		auto pSendOverlapped = new MyOverlapped();
		pSendOverlapped->coTask = PostSend(pSendOverlapped);
		pSendOverlapped->coTask.Run();
	}
	{
		auto pOverlapped = new MyOverlapped();
		//新客户端投递recv
		PostRecv(pOverlapped);
	}
	return;
}

inline CoTask<int> SessionSocketCompeletionKey::PostRecv(MyOverlapped* pOverlapped)
{
	WSABUF wsabuf;
	wsabuf.buf = recv_buf;
	wsabuf.len = MAX_RECV_COUNT;

	DWORD dwRecvCount;
	DWORD dwFlag = 0;

	int nRes = WSARecv(Socket(), &wsabuf, 1, &dwRecvCount, &dwFlag, &pOverlapped->overlapped, NULL);

	int a = WSAGetLastError();
	if (ERROR_IO_PENDING == a)
	{
		return true;
	}

	if (a == 0)//同步操作成功
	{
		PostRecv(pOverlapped);
		return true;
	}
	printf("PostRecv err");
	return false;
}

/// <summary>
/// 
/// </summary>
/// <param name="pOverlapped"></param>
/// <param name="refSize"></param>
/// <returns>是否调用了WSASend，是否同步返回</returns>
std::tuple<bool, bool>  SessionSocketCompeletionKey::WSASend(MyOverlapped* pOverlapped)
{
	DWORD dwSendCount(0);
	DWORD dwFlag = 0;
	std::tie(pOverlapped->wsabuf.buf, pOverlapped->wsabuf.len) = this->sendBuf.BuildSendBuf();
	pOverlapped->GetQueuedCompletionStatusReturn = ::WSASend(Socket(), &pOverlapped->wsabuf, 1, &dwSendCount, dwFlag, &pOverlapped->overlapped, NULL);

	pOverlapped->GetLastErrorReturn = WSAGetLastError();
	pOverlapped->numberOfBytesTransferred = dwSendCount;
	if (pOverlapped->GetQueuedCompletionStatusReturn) 
	{
		return std::make_tuple(false, false);
	}
	if (pOverlapped->GetLastErrorReturn == 0)
	{
		//同步发送完成
		//PostSend(pOverlapped);
		return std::make_tuple(true, true);
	}
	if (ERROR_IO_PENDING != pOverlapped->GetLastErrorReturn)
	{
		//延迟处理
		//函数执行出错
		switch (a) {
		case WSAENOTCONN:
			printf("A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied.\n");
			break;
		}
		/*return false;*/
	}
	return std::make_tuple(true, false);
}
CoTask<int> SessionSocketCompeletionKey::PostSend(MyOverlapped* pOverlapped)
{
	
	while (true) 
	{
		bool callSend, sync;
		std::tie(callSend, sync) = WSASend(pOverlapped);
		if (!callSend)
		{
			co_yield 0;
			continue;
		}

		if (sync)
			co_yield 0;
	
		this->sendBuf.SendComplete(pOverlapped->numberOfBytesTransferred);
	}
}
