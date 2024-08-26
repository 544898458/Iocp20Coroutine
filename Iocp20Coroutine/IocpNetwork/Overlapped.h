#pragma once
#include <Winsock2.h>
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
		Overlapped() :coAwait(0, funCancel), OnComplete(&Overlapped::OnCompleteNormal){}
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
		bool callSend = false;
		/// <summary>
		/// 只在连接Socket(Session)里用到。监听Socket(Accept)不会用到
		/// </summary>
		WSABUF wsabuf = { 0,nullptr };
		enum SendState
		{
			SendState_Sleep,//沉睡，随时唤醒
			SendState_Sending,//忙着发数据，不需要通知，只要王队列里塞数据就行了
			SendState_SendBeforeSleep,//睡眠前最后一批发送，要通知，但是收到通知不一定执行协程（可能老协程还在运行）
		};
		/// <summary>
		/// 发送停止
		/// </summary>
		std::atomic<SendState> atomicSendState = SendState_Sleep;
		void (Overlapped::*OnComplete)(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr);
		void OnCompleteSend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->pCoTask->m_mutex);//能进这个锁，说明协程肯定是挂起状态

			this->numberOfBytesTransferred = number_of_bytes;
			this->GetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
			this->GetLastErrorReturn = lastErr;
			if (nullptr != this->pCoTask)//通知唤醒
			{
				{
					SendState finalSendRunning = SendState_SendBeforeSleep;//如果正在停止，就恢复运行
					const bool changed = atomicSendState.compare_exchange_strong(finalSendRunning, SendState_Sending);
					return;
				}

				SendState finalSendRunning = SendState_Sleep;//如果已经睡眠，就立刻唤醒
				const bool changed = atomicSendState.compare_exchange_strong(finalSendRunning, SendState_Sending);//先标记运行
				if (changed)
				{
					LOG(INFO) << "唤醒沉睡的Send协程";
					this->pCoTask->Run();
				}
				else
				{
					assert(Sending == finalSendRunning);
					LOG(INFO) << "唤醒沉睡的Send协程已被其它操作唤醒";
				}
				return;
			}
		}
		void OnCompleteNotifySend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->pCoTask->m_mutex);//能进这个锁，说明协程肯定是挂起状态
			switch (this->pCoTask->GetValue())
			{
			case Sending:
				LOG(INFO) << "正在发送";
				break;
			case SendStop:
			{
				{
					auto finalSendState = SendState_Sending;
					const bool changed = atomicSendState.compare_exchange_strong(finalSendState, SendState_SendBeforeSleep);//睡前再操作一次
					if (changed)
					{
						LOG(INFO) << "Send协程沉睡前再试一次";
						PostQueuedCompletionStatus(port, 0, (ULONG_PTR)pKey, &overlapped);
						return;
					}
				}
				
				{
					auto finalSendState = SendState_SendBeforeSleep;
					const bool changed = atomicSendState.compare_exchange_strong(finalSendState, SendState_Sleep);//标记已停止
					LOG(INFO) << "Send协程沉睡前再试一次也结束了";
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