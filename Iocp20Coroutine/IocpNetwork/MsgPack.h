#pragma once
#include <sstream>
#include <msgpack.hpp>
#include "../LogStrategy/StrategyLog.h"
#include "ByteQueue.h"

namespace MsgPack
{
	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="T_Msg">����Ϣ��MsgPack���л�</typeparam>
	/// <typeparam name="FUN"></typeparam>
	/// <param name="ref"></param>
	/// <param name="refFun"></param>
	template<class T_Msg, typename FUN>
	static void SendMsgpack(const T_Msg& ref, const FUN& refFun, const bool bWriteSize = true)
	{
		//��ͨ�ṹ���л�����ѹ���޼���
		std::stringstream buffer;
		msgpack::pack(buffer, ref);
		buffer.seekg(0);

		std::string str(buffer.str());
		CHECK_GE_VOID(UINT16_MAX, str.size());
		const uint16_t usSize = (uint16_t)str.size();
		if (!bWriteSize)
		{
			refFun(str.data(), usSize);
			return;
		}

		//ByteQueueSend sendBuff;
		//sendBuff.queue.Enqueue(&usSize, sizeof(usSize));
		//sendBuff.queue.Enqueue(str.data(), usSize);
		//const auto&& [bufAll, lenAll] = sendBuff.BuildSendBuf();
		//refFun(bufAll, lenAll);
		//�������̫����
		static std::vector<char> vec(1024 * 16);
		vec.resize(sizeof(usSize) + usSize);
		*(uint16_t*)&vec[0] = usSize;
		memcpy(&vec[2], str.data(), usSize);
		refFun(&vec[0], (int)vec.size());
	}
};

