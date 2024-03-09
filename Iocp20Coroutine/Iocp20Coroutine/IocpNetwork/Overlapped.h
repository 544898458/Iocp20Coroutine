#pragma once
#include "../CoRoutine/CoTask.h"
class SocketCompeletionKey;
namespace Iocp 
{
	//constexpr int MAX_SEND_COUNT = 2048;
	/// <summary>
	/// 重叠操作，对应一次Accet、Recv、Send
	/// CONTAINING_RECORD（）可以根据Overlapped获取PER_IO_CONTEXT数据类型。前提是Overlapped是PER_IO_CONTEXT的第一个成员。CONTAINING_RECORD可以根据Overlapped的地址获取其所在PER_IO_CONTEXT结构体的地址指针。
	///	PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(lpOverlapped, PER_IO_CONTEXT, m_Overlapped);
	/// </summary>
	struct Overlapped
	{
		//enum Op
		//{
		//	Accept,
		//	Recv,
		//	Send,
		//};

		OVERLAPPED overlapped = { 0 };


		SOCKET socket = NULL;

		//完成端口返回结果，GetQueuedCompletionStatus 返回结果
		DWORD numberOfBytesTransferred;
		BOOL GetQueuedCompletionStatusReturn;
		int GetLastErrorReturn;
		DWORD dwSendCount;
		bool callSend;
		/// <summary>
		/// 只在连接Socket(Session)里用到。监听Socket(Accept)不会用到
		/// </summary>
		WSABUF wsabuf = { 0,nullptr };

		void OnComplete(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			this->numberOfBytesTransferred = number_of_bytes;
			this->GetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
			this->GetLastErrorReturn = lastErr;
			this->coTask.Run();
		}
		CoTask<int> coTask;
		bool needDeleteMe=false;
	};
}