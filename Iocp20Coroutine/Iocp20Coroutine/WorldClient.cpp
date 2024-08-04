#include "StdAfx.h"
#include "WorldClient.h"

template Iocp::Server<WorldClient>;
//template bool Iocp::Server<WorldClient>::Init<WorldClientSession>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<WorldClientSession>;
std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;

void WorldClientSession::OnRecvPack(const void* buf, int len)
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
		LOG(INFO) << "WorldSvr发来聊天:" << StrConv::Utf8ToGbk(msg.content);

		this->m_pWorldClient->m_funBroadcast(msg);
	}
	break;
	}
}
