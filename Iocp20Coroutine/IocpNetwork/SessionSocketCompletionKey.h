#pragma once
#include "SocketCompeletionKey.h"
#include<WinSock2.h>
#include <set>
#include "ByteQueue.h"
namespace Iocp 
{
	template<class T_Session>
	class SessionSocketCompletionKey :public SocketCompeletionKey
	{
	public:
		SessionSocketCompletionKey(SOCKET s) :SocketCompeletionKey(s)
		{

		}
		virtual ~SessionSocketCompletionKey();
		void StartCoRoutine();
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
		std::mutex lockFinish;
		bool sendFinish = false;
		bool recvFinish = false;
	};

	static std::tuple< const void*, int > OnRecv2(const void* buf, int len)
	{
		uint16_t usPackLen(0);
		const auto sizeofPackLen = sizeof(usPackLen);
		if (sizeofPackLen > len)
			return std::make_tuple(nullptr, 0);

		usPackLen = *(uint16_t*)buf;
		if (usPackLen > 8192)
		{
			LOG(ERROR) << "���,�������ݰ�len=" << len;
			return std::make_tuple(nullptr, len);
		}
		if (usPackLen + sizeofPackLen > len)
		{
			//LOG(INFO) << "ϣ������" << usPackLen << "+" << sizeofPackLen << "�ֽ�,�����յ�" << len << "�ֽ�,�´λ�Ҫ������";
			return std::make_tuple(nullptr, 0);
		}

		//OnRecvPack((const void*)buf + sizeofPackLen, len - sizeofPackLen);
		return std::make_tuple((const char*)buf + sizeofPackLen, len);
	}
}
