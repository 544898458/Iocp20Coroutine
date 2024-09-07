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
		Overlapped() :coAwait(0, funCancel), OnComplete(&Overlapped::OnCompleteNormal) {}
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
		void (Overlapped::* OnComplete)(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr);
		void OnCompleteSend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->coTask.m_mutex);//�ܽ��������˵��Э�̿϶��ǹ���״̬

			this->numberOfBytesTransferred = number_of_bytes;
			this->GetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
			this->GetLastErrorReturn = lastErr;
			if (this->coTask.Run())
			{
				LOG(INFO) << "Э�̽���";
				return;
			}
			auto yiledValue = this->coTask.GetValue();
			switch (yiledValue)
			{
			case Sending:
				//LOG(INFO) << "���ڷ���";
				break;
			case SendStop:
			{
				auto finalSendState = SendState_SendBeforeSleep;
				auto changed = this->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sleep);
				if (changed)
				{
					//LOG(INFO) << "SendЭ�̳�˯ǰ����һ��Ҳ������,Э�̽���˯��״̬";
					return;
				}

				finalSendState = SendState_Sending;
				changed = this->atomicSendState.compare_exchange_strong(finalSendState, SendState_SendBeforeSleep);//˯ǰ�ٲ���һ��
				assert(changed);
				if (!changed)
				{
					LOG(ERROR) << "error,changed=" << changed << ",finalSendState=" << finalSendState;
					return;
				}

				//LOG(INFO) << "SendЭ��˯ǰ�ٲ���һ�Σ�changed=" << changed << ",finalSendState=" << finalSendState;
				PostQueuedCompletionStatus(port, 0, (ULONG_PTR)pKey, &this->overlapped);
			}
			break;
			default:
				LOG(ERROR) << "yiledValue=" << yiledValue;
				assert(false);
				return;// break;
			}
		}

		void OnCompleteNotifySend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->pOverlapped->coTask.m_mutex);//�ܽ��������˵��Э�̿϶��ǹ���״̬

			const auto yiledValue = this->pOverlapped->coTask.GetValue();
			switch (yiledValue)
			{
			case Sending:
				//LOG(INFO) << "���ڷ��ͣ���������";
				break;
			case SendStop:
			{
				auto finalSendState = SendState_Sleep;
				auto changed = this->pOverlapped->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sending);//����
				if (changed)
				{
					//LOG(INFO) << "�����˯��SendЭ��";
					PostQueuedCompletionStatus(port, 0, (ULONG_PTR)pKey, &this->pOverlapped->overlapped);
					return;
				}
				if (finalSendState == SendState_Sending)
				{
					//LOG(INFO) << "SendЭ�����ڵȴ����ͽ��";
					return;
				}
				assert(finalSendState == SendState_SendBeforeSleep);
				if (finalSendState != SendState_SendBeforeSleep)
				{
					LOG(ERROR) << "ERROR";
					return;
				}
				changed = this->pOverlapped->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sending);//����
				assert(changed);
				if (changed)
				{
					LOG(INFO) << "SendState_SendBeforeSleep,ת,SendState_Sending";
					return;
				}

				LOG(ERROR) << "changed=" << changed << ",finalSendState=" << finalSendState;
			}
			break;
			default:
				LOG(ERROR) << "yiledValue=" << yiledValue;
				assert(false);
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
		Overlapped* pOverlapped = nullptr;
		FunCancel funCancel;
		CoAwaiterBool coAwait;
		bool needDeleteMe = false;
	};
}