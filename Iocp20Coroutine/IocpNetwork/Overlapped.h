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
		Overlapped(const std::string& strDebugInfo = "") :coAwait(0, funCancel, strDebugInfo), OnComplete(&Overlapped::OnCompleteNormal), funCancel(strDebugInfo){}
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
		//bool callSend = false;
		/// <summary>
		/// 只在连接Socket(Session)里用到。监听Socket(Accept)不会用到
		/// </summary>
		WSABUF wsabuf = { 0,nullptr };
		char bufOutput[1024];
		DWORD dwRecvcount = 0;
		DWORD dwFlag = 0;
		enum SendState
		{
			SendState_Sleep,//沉睡，随时唤醒
			SendState_Sending,//忙着发数据，不需要通知，只要往队列里塞数据就行了
			SendState_SendBeforeSleep,//睡眠前最后一批发送，要通知，但是收到通知不一定执行协程（可能老协程还在运行）
		};
		/// <summary>
		/// 发送停止
		/// </summary>
		std::atomic<SendState> atomicSendState = SendState_Sleep;
		void (Overlapped::* OnComplete)(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr);
		void OnCompleteSend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->coTask.m_mutex);//能进这个锁，说明协程肯定是挂起状态

			this->numberOfBytesTransferred = number_of_bytes;
			this->GetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
			this->GetLastErrorReturn = lastErr;

			if (this->coTask.Run())
			{
				LOG(INFO) << "协程结束";
				LOG_IF(WARNING, OK != this->coTask.GetValue()) << "停止send";

				LOG(INFO) << "Send协程结束，肯定断线了";
				auto finalSendState = SendState_Sending;
				const auto old = this->atomicSendState.load();
				auto changed = this->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sleep);
				if (changed)
				{
					LOG(INFO) << "Send协程结束，SendState_Sending时断线,finalSendState:" << finalSendState << ",old=" << old;
				}
				else if (SendState_SendBeforeSleep == old)
				{
					LOG(WARNING) << "Send协程结束，SendState_SendBeforeSleep时断线,finalSendState:" << finalSendState << ",old=" << old;
					changed = this->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sleep);
					if (!changed)
					{
						LOG(ERROR) << "Send协程结束,状态有问题,finalSendState:" << finalSendState << ",old=" << old;
					}
				}

				return;
			}
			auto yiledValue = this->coTask.GetValue();
			switch (yiledValue)
			{
			case Sending:
				//LOG(INFO) << "正在发送";
				break;
			case SendStop:
			{
				auto finalSendState = SendState_SendBeforeSleep;
				auto changed = this->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sleep);
				if (changed)
				{
					//LOG(INFO) << "Send协程沉睡前再试一次也结束了,协程进入睡眠状态";
					return;
				}

				finalSendState = SendState_Sending;
				changed = this->atomicSendState.compare_exchange_strong(finalSendState, SendState_SendBeforeSleep);//睡前再操作一次
				LOG_IF(ERROR, !changed) << "";
				_ASSERT(changed);
				if (!changed)
				{
					LOG(ERROR) << "error,changed=" << changed << ",finalSendState=" << finalSendState;
					return;
				}

				//LOG(INFO) << "Send协程睡前再操作一次，changed=" << changed << ",finalSendState=" << finalSendState;
				PostQueuedCompletionStatus(port, 0, (ULONG_PTR)pKey, &this->overlapped);
			}
			break;
			default:
				LOG(ERROR) << "yiledValue=" << yiledValue;
				_ASSERT(false);
				return;// break;
			}
		}

		void OnCompleteNotifySend(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
		{
			std::lock_guard lock(this->pOverlapped->coTask.m_mutex);//能进这个锁，说明协程肯定是挂起状态

			if (this->pOverlapped->coTask.Finished())
			{
				LOG(WARNING) << "协程已结束，不能再激活发送协程,atomicSendState=" << this->pOverlapped->atomicSendState.load();
				return;
			}
			const auto yiledValue = this->pOverlapped->coTask.GetValue();
			switch (yiledValue)
			{
			case OK:
				LOG(INFO) << "已经停止发送，不用再次启动发送";
				break;
			case Sending:
				//LOG(INFO) << "正在发送，不用启动";
				break;
			case SendStop:
			{
				auto finalSendState = SendState_Sleep;
				auto changed = this->pOverlapped->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sending);//激活
				if (changed)
				{
					//LOG(INFO) << "激活沉睡的Send协程";
					PostQueuedCompletionStatus(port, 0, (ULONG_PTR)pKey, &this->pOverlapped->overlapped);
					return;
				}
				if (finalSendState == SendState_Sending)
				{
					//LOG(INFO) << "Send协程正在等待发送结果";
					return;
				}
				LOG_IF(ERROR, finalSendState != SendState_SendBeforeSleep) << "";
				_ASSERT(finalSendState == SendState_SendBeforeSleep);
				if (finalSendState != SendState_SendBeforeSleep)
				{
					LOG(ERROR) << "ERROR";
					return;
				}
				changed = this->pOverlapped->atomicSendState.compare_exchange_strong(finalSendState, SendState_Sending);//激活
				LOG_IF(ERROR, !changed) << "";
				_ASSERT(changed);
				if (changed)
				{
					//LOG(INFO) << "SendState_SendBeforeSleep,转,SendState_Sending";
					return;
				}

				LOG(ERROR) << "changed=" << changed << ",finalSendState=" << finalSendState;
			}
			break;
			default:
				LOG(ERROR) << "yiledValue=" << yiledValue;
				_ASSERT(false);
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
		FunCancel安全 funCancel;
		CoAwaiterBool coAwait;
		bool needDeleteMe = false;
	};
}