#include "pch.h"
#include "WorldClient.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "GateServer.h"
//template Iocp::Server<WorldClient>;
//template bool Iocp::Server<WorldClient>::Init<WorldClientSession>(const uint16_t);
//template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>;
//std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;

/// <summary>
/// 主线程，单线程
/// </summary>
void ClientSession_GateToWorld::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//没有消息可处理
			break;

		switch (msgId)
		{
		case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &ClientSession_GateToWorld::OnRecv); break;
		case MsgId::Gate转发:this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &ClientSession_GateToWorld::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}
}


void ClientSession_GateToWorld::OnRecv(const MsgLogin& msg)
{
	
}

void ClientSession_GateToWorld::OnRecv(const MsgGate转发& msg转发)
{
	if (msg转发.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}

	msgpack::object_handle oh = msgpack::unpack((const char*)&msg转发.vecByte[0], msg转发.vecByte.size());//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	LOG(INFO) << obj;
	extern std::unique_ptr<Iocp::Server<GateServer>> g_upGateSvr;
	auto pSession = g_upGateSvr->m_Server.m_Sessions.GetSession(msg转发.gateClientSessionId);
	//auto pSession = (GateSession*)gateSessionId;
	CHECK_NOTNULL_VOID(pSession);
	
	pSession->Session.m_Session.OnRecvWorldSvr(obj.as<MsgLoginResponce>());
}

template<> std::deque<MsgLogin>& ClientSession_GateToWorld::GetQueue() { return m_queueLogin; }
template<> std::deque<MsgGate转发>& ClientSession_GateToWorld::GetQueue() { return m_queueGate转发; }

/// <summary>
/// 网络线程（多线程）调用
/// </summary>
/// <param name="refSession"></param>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <returns>返回已处理的字节数，这些数据将立刻从接受缓冲中删除</returns>

inline int ClientSession_GateToWorld::OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>& refSession, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &ClientSession_GateToWorld::OnRecvPack);
}

/// <summary>
/// 网络线程（多线程）调用
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void ClientSession_GateToWorld::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
	case MsgId::Login:	m_MsgQueue.PushMsg<MsgLogin>(*this,obj);break;
	case MsgId::Gate转发:	m_MsgQueue.PushMsg<MsgGate转发>(*this, obj); break;
	default:
		LOG(WARNING) << "ERR:" << msg.id;
		break;
	}
}