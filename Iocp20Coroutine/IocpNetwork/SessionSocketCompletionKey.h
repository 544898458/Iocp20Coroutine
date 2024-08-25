#pragma once
#include "SocketCompeletionKey.h"
#include<WinSock2.h>
#include <set>
#include "ByteQueue.h"
namespace Iocp
{
	template<class T_Session>
	//requires requires(T_Session &refSession, Iocp::SessionSocketCompletionKey<T_Session> &refSessionSocketCompletionKey)
	//{
	//	requires std::is_same_v<int, decltype(refSession.OnRecv(refSessionSocketCompletionKey, (const void*)0, (int)0))>;
	//}
	class SessionSocketCompletionKey :public SocketCompeletionKey
	{
	public:
		SessionSocketCompletionKey(SOCKET s) :SocketCompeletionKey(s), Session(*this)
		{

		}
		virtual ~SessionSocketCompletionKey();
		void StartCoRoutine(HANDLE hIocp);
		void Send(const void* buf, int len);
		bool Finished();
		T_Session Session;
	private:
		CoTask<int> PostSend(Overlapped& pOverlapped);
		CoTask<int> PostRecv(Overlapped& pOverlapped);
		std::tuple<bool, bool>  WSASend(Overlapped& pOverlapped);
		bool WSARecv(Overlapped& pOverlapped);

	private:
		ByteQueueSend sendBuf;
		ByteQueueRecv recvBuf;
		Overlapped sendOverlapped;
		Overlapped recvOverlapped;
		//std::atomic_bool atomicWaitingSendResult = false;
		std::mutex lockFinish;
		bool sendFinish = false;
		bool recvFinish = false;
		int maxSendQueue = 0;
		HANDLE m_hIocp;
	};

	static std::tuple< const void*, int, int > OnRecv2(const void* buf, int len)
	{
		uint16_t usPackLen(0);
		const auto sizeofPackLen = sizeof(usPackLen);
		if (sizeofPackLen > len)
			return std::make_tuple(nullptr, 0, len);

		usPackLen = *(uint16_t*)buf;
		if (usPackLen > 8192)
		{
			LOG(ERROR) << "外挂,跳过数据包len=" << len;
			return std::make_tuple(nullptr, 0, len);
		}
		if (usPackLen + sizeofPackLen > len)
		{
			//LOG(INFO) << "希望接收" << usPackLen << "+" << sizeofPackLen << "字节,现已收到" << len << "字节,下次还要接着收";
			return std::make_tuple(nullptr, 0, 0);
		}

		//OnRecvPack((const void*)buf + sizeofPackLen, len - sizeofPackLen);
		return std::make_tuple((const char*)buf + sizeofPackLen, usPackLen, usPackLen + sizeofPackLen);
	}
	template<class T>
	static int OnRecv3(const void* buf, const int len, T&ref, void (T::*OnRecvPack)(const void*, const int))
	{
		int processedLenAll = 0;
		const char* bufIter = (const char*)buf;
		int lenIter = len;
		while(true)
		{
			auto [bufPack, lenPack, processedLen] = OnRecv2(bufIter, lenIter);
			if (lenPack <= 0 || nullptr == bufPack)
				return processedLenAll;

			(ref.*OnRecvPack)(bufPack, lenPack);
			bufIter += processedLen;
			lenIter -= processedLen;
			processedLenAll += processedLen;
		}
		return processedLenAll;
	}
}
