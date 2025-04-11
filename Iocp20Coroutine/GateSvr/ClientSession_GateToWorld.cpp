#include "pch.h"
#include "ClientSession_GateToWorld.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "GateServer.h"

template Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>;

/// <summary>
/// 主线程，单线程
/// </summary>
void ClientSession_GateToWorld::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::MsgId_Invalid_0 == msgId)//没有消息可处理
			break;

		switch (msgId)
		{
		case MsgId::Gate转发:this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &ClientSession_GateToWorld::OnRecv); break;
		case MsgId::在线人数:this->m_MsgQueue.OnRecv(this->m_queue在线人数, *this, &ClientSession_GateToWorld::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			_ASSERT(false);
			break;
		}
	}
}

void ClientSession_GateToWorld::OnRecv(const MsgGate转发& msg转发)
{
	if (msg转发.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		_ASSERT(false);
		return;
	}

	msgpack::object_handle oh = msgpack::unpack((const char*)&msg转发.vecByte[0], msg转发.vecByte.size());//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	LOG(INFO) << obj;
	extern std::unique_ptr<Iocp::Server<GateServer>> g_upGateSvr;
	if (!g_upGateSvr->m_Server.m_Sessions.GetSession(msg转发.gateClientSessionId, [&msg, &obj](auto& refGateSession) {
		switch (msg.id)
		{
		case MsgId::Login:	refGateSession.Session.m_Session.OnRecvWorldSvr(obj.as<MsgLoginResponce>());	break;
		case MsgId::GateDeleteSession:	refGateSession.Session.m_Session.OnRecvWorldSvr(obj.as<MsgGateDeleteSession>());	break;
		}
		}))
	{
		switch (msg.id)
		{
		case MsgId::GateDeleteSession:
			LOG(WARNING) << "WorldSvr 请求 GateSvr 删除玩家 GateSession 时,此 Session 早已断线,直接返回删除成功,gateClientSessionId=" << msg转发.gateClientSessionId;
			void SendResponceToWorldSvr(const uint32_t rpcSnId, const uint64_t gateSessionId);
			SendResponceToWorldSvr(msg.rpcSnId, msg转发.gateClientSessionId);
			break;
		default:
			LOG(ERROR) << msg.id << "=msg.id,msg转发.gateClientSessionId=" << msg转发.gateClientSessionId;
			break;
		}
	}
}
void BroadToGateClient(const MsgGateSvr转发WorldSvr消息给游戏前端& refMsg);
void ClientSession_GateToWorld::OnRecv(const Msg在线人数& msg在线人数)
{
	MsgPack::SendMsgpack(msg在线人数, [](const void* buf, int len)
		{
			MsgGateSvr转发WorldSvr消息给游戏前端 msg给前端(buf, len);
			BroadToGateClient(msg给前端);
		}, false);

}

template<> std::deque<MsgGateDeleteSession>& ClientSession_GateToWorld::GetQueue() { return m_queueGateDeleteSession; }
template<> std::deque<MsgGate转发>& ClientSession_GateToWorld::GetQueue() { return m_queueGate转发; }
template<> std::deque<Msg在线人数>& ClientSession_GateToWorld::GetQueue() { return m_queue在线人数; }

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
	try
	{
		msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
		msgpack::object obj = oh.get();
		const auto msg = MsgHead::GetMsgId(obj);
		//LOG(INFO) << obj;

		switch (msg.id)
		{
		case MsgId::GateDeleteSession:	m_MsgQueue.PushMsg<MsgGateDeleteSession>(*this, obj); break;
		case MsgId::Gate转发:	m_MsgQueue.PushMsg<MsgGate转发>(*this, obj); break;
		case MsgId::在线人数:	m_MsgQueue.PushMsg<Msg在线人数>(*this, obj); break;
		default:
			LOG(WARNING) << "ERR:" << msg.id;
			break;
		}
	}
	catch (const msgpack::type_error& error)
	{
		LOG(ERROR) << len << "=len, ClientSession_GateToWorld,反序列化失败," << error.what();
		_ASSERT(false);
		return;
	}
}