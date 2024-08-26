#pragma once
#include <Winsock2.h>
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
		Overlapped() :coAwait(0, funCancel), OnComplete(&Overlapped::OnCompleteNormal){}
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
		bool callSend = false;
		/// <summary>
		/// ֻ������Socket(Session)���õ�������Socket(Accept)�����õ�
		/// </summary>
		WSABUF wsabuf = { 0,nullptr };
		enum SendState
		{
			SendState_Sleep,//��˯����ʱ����
			SendState_Sending,//æ�ŷ����ݣ�����Ҫ֪ͨ��ֻҪ�������������ݾ�����
			SendState_SendBeforeSleep,//˯��ǰ���һ�����ͣ�Ҫ֪ͨ�������յ�֪ͨ��һ��ִ��Э�̣�������Э�̻������У�
		};
		/// <summary>
		/// ����ֹͣ
		/// </summary>
		std::atomic<SendState> atomicSendState = SendState_Sleep;
		void (Overlapped::*OnComplete)(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr);
		void OnCompleteSend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->pCoTask->m_mutex);//�ܽ��������˵��Э�̿϶��ǹ���״̬

			this->numberOfBytesTransferred = number_of_bytes;
			this->GetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
			this->GetLastErrorReturn = lastErr;
			if (nullptr != this->pCoTask)//֪ͨ����
			{
				{
					SendState finalSendRunning = SendState_SendBeforeSleep;//�������ֹͣ���ͻָ�����
					const bool changed = atomicSendState.compare_exchange_strong(finalSendRunning, SendState_Sending);
					return;
				}

				SendState finalSendRunning = SendState_Sleep;//����Ѿ�˯�ߣ������̻���
				const bool changed = atomicSendState.compare_exchange_strong(finalSendRunning, SendState_Sending);//�ȱ������
				if (changed)
				{
					LOG(INFO) << "���ѳ�˯��SendЭ��";
					this->pCoTask->Run();
				}
				else
				{
					assert(Sending == finalSendRunning);
					LOG(INFO) << "���ѳ�˯��SendЭ���ѱ�������������";
				}
				return;
			}
		}
		void OnCompleteNotifySend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->pCoTask->m_mutex);//�ܽ��������˵��Э�̿϶��ǹ���״̬
			switch (this->pCoTask->GetValue())
			{
			case Sending:
				LOG(INFO) << "���ڷ���";
				break;
			case SendStop:
			{
				{
					auto finalSendState = SendState_Sending;
					const bool changed = atomicSendState.compare_exchange_strong(finalSendState, SendState_SendBeforeSleep);//˯ǰ�ٲ���һ��
					if (changed)
					{
						LOG(INFO) << "SendЭ�̳�˯ǰ����һ��";
						PostQueuedCompletionStatus(port, 0, (ULONG_PTR)pKey, &overlapped);
						return;
					}
				}
				
				{
					auto finalSendState = SendState_SendBeforeSleep;
					const bool changed = atomicSendState.compare_exchange_strong(finalSendState, SendState_Sleep);//�����ֹͣ
					LOG(INFO) << "SendЭ�̳�˯ǰ����һ��Ҳ������";
				}
			}
			break;
			default:
				break;
			}

			//this->coAwait.Run2(nullptr!=pKey, this->coTask.m_mutex);
		}
		void OnCompleteNormal(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			this->numberOfBytesTransferred = number_of_bytes;
			this->GetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
			this->GetLastErrorReturn = lastErr;
			this->coTask.Run();
		}
		CoAwaiterBool& WaitSendResult()
		{
			return coAwait;
		}
		enum YieldReturn
		{
			OK,
			Error,
			Sending,
			SendStop
		};
		CoTask<YieldReturn> coTask;
		CoTask<YieldReturn>* pCoTask = nullptr;
		FunCancel funCancel;
		CoAwaiterBool coAwait;
		bool needDeleteMe = false;
	};
}