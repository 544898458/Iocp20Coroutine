#pragma once
#include "../CoRoutine/CoTask.h"
class SocketCompeletionKey;
namespace Iocp 
{
	//constexpr int MAX_SEND_COUNT = 2048;
	/// <summary>
	/// �ص���������Ӧһ��Accet��Recv��Send
	/// CONTAINING_RECORD�������Ը���Overlapped��ȡPER_IO_CONTEXT�������͡�ǰ����Overlapped��PER_IO_CONTEXT�ĵ�һ����Ա��CONTAINING_RECORD���Ը���Overlapped�ĵ�ַ��ȡ������PER_IO_CONTEXT�ṹ��ĵ�ַָ�롣
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

		//��ɶ˿ڷ��ؽ����GetQueuedCompletionStatus ���ؽ��
		DWORD numberOfBytesTransferred;
		BOOL GetQueuedCompletionStatusReturn;
		int GetLastErrorReturn;
		DWORD dwSendCount;
		bool callSend;
		/// <summary>
		/// ֻ������Socket(Session)���õ�������Socket(Accept)�����õ�
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