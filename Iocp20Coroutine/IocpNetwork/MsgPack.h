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
	/// <typeparam name="T_Msg">此消息用MsgPack序列化</typeparam>
	/// <typeparam name="FUN"></typeparam>
	/// <param name="ref"></param>
	/// <param name="refFun"></param>
	template<class T_Msg, typename FUN>
	static void SendMsgpack(const T_Msg& ref, const FUN& refFun, const bool bWriteSize = true)
	{
		//普通结构序列化，无压缩无加密
		std::stringstream buffer;
		msgpack::pack(buffer, ref);
		buffer.seekg(0);

		std::string str(buffer.str());
		CHECK_GE_VOID(UINT16_MAX, str.size());
		const uint16_t usSize = (uint16_t)str.size();
		ByteQueueSend sendBuff;
		if (bWriteSize)
			sendBuff.queue.Enqueue(&usSize, sizeof(usSize));

		sendBuff.queue.Enqueue(str.data(), usSize);
		const auto&& [bufAll, lenAll] = sendBuff.BuildSendBuf();
		refFun(bufAll, lenAll);
	}
};

