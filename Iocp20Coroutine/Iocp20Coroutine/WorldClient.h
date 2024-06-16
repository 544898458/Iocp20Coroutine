#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
#include <msgpack.hpp>
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

enum MsgId;
class WorldClient;
class WorldClientSession
{
public:
	void OnInit(Iocp::SessionSocketCompeletionKey<WorldClientSession>& session, WorldClient&)
	{

	}

	/// <summary>
	///
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns>返回已处理的字节数，这些数据将立刻从接受缓冲中删除</returns>
	int OnRecv(Iocp::SessionSocketCompeletionKey<WorldClientSession>& refSession, const void* buf, int len)
	{
		uint16_t usPackLen(0);
		const auto sizeofPackLen = sizeof(usPackLen);
		if (sizeofPackLen > len)
			return 0;

		usPackLen = *(uint16_t*)buf;
		if (usPackLen > 8192)
		{
			LOG(ERROR) << "跳过数据包len=" << len;
			return len;
		}
		if (usPackLen + sizeofPackLen > len)
			return 0;

		OnRecvPack((const char*)buf + sizeofPackLen, len - sizeofPackLen);
		return len;
	}
	void OnRecvPack(const void* buf, int len)
	{
		msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
		msgpack::object obj = oh.get();
		const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//没判断越界，要加try
		LOG(INFO) << obj;

		switch (msgId)
		{
		case MsgId::Say:
		{
			const auto msg = obj.as<MsgSay>();
			LOG(INFO) << StrConv::Utf8ToGbk(msg.content);
		}
		break;
		}
	}
	void OnDestroy()
	{

	}
};
class WorldClient
{
public:
	void OnAdd(Iocp::SessionSocketCompeletionKey<WorldClientSession>& session)
	{

	}

};
