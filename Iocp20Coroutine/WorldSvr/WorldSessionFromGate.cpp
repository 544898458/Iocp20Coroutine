#include "pch.h"
#include "WorldSessionFromGate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "WorldSvrAcceptGate.h"
//#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "../CoRoutine/CoDb.h"
#include "PlayerGateSession_World.h"
#include "DbPlayer.h"

template class Iocp::SessionSocketCompletionKey<WorldSessionFromGate>;
template class MsgQueueMsgPack<WorldSessionFromGate>;
//template class WebSocketSession<WorldSession>;
//template void WebSocketSession<WorldSession>::OnInit<WorldServer>(Iocp::SessionSocketCompletionKey<WebSocketSession<WorldSession> >& refSession, WorldServer& server);
//template class WebSocketEndpoint<WorldSession, Iocp::SessionSocketCompletionKey<WebSocketSession<WorldSession> > >;

/// <summary>
/// 网络线程，多线程
/// </summary>
/// <param name=""></param>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <returns></returns>
int WorldSessionFromGate::OnRecv(CompeletionKeySession&, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &WorldSessionFromGate::OnRecvPack);
}

bool WorldSessionFromGate::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		switch (msgId)
		{
		case MsgId::MsgId_Invalid_0://没有消息可处理
			return true;
			//case MsgId::Login:	this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &WorldSession::OnRecv); break;
		case MsgId::Gate转发:	return this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &WorldSessionFromGate::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			return false;
			break;
		}
	}

	assert(false);
	return false;
}

/// <summary>
/// 网络线程，多线程
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void WorldSessionFromGate::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
	case MsgId::Gate转发:m_MsgQueue.PushMsg<MsgGate转发>(*this, obj); break;
	default:
		LOG(WARNING) << "没处理GameSvr发来的消息:" << msg.id;
		break;
	}
}

std::map<uint64_t, PlayerGateSession_World> g_mapPlayerGateSession;
extern std::map<std::string, uint64_t> g_mapPlayerNickNameGateSessionId;
void WorldSessionFromGate::OnRecv(const MsgGate转发& msg转发)
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

	auto iter = g_mapPlayerGateSession.find(msg转发.gateClientSessionId);
	switch (msg.id)
	{
	case MsgId::GateDeleteSession:
	{
		//assert(g_mapPlayerGateSession.end() != iter);
		if (g_mapPlayerGateSession.end() == iter)
		{
			LOG(WARNING) << "不符合协议的黑客攻击";//可用telnet向端口发数据模拟
			return;
		}
		auto& refPlayerGateSession = iter->second;
		{
			auto iterFind = g_mapPlayerNickNameGateSessionId.find(refPlayerGateSession.NickName());
			assert(g_mapPlayerNickNameGateSessionId.end() != iterFind);
			if (g_mapPlayerNickNameGateSessionId.end() != iterFind)
			{
				LOG(INFO) << refPlayerGateSession.NickName() << ",删除名字对应的SessionId:" << msg转发.gateClientSessionId;
				g_mapPlayerNickNameGateSessionId.erase(iterFind);
			}
		}
		g_mapPlayerGateSession.erase(iter);
		LOG(INFO) << "删除PlayerGateSession_World对象,Id=" << msg转发.gateClientSessionId;
		
	}
	break;
	default:
	{
		if (g_mapPlayerGateSession.end() == iter)
		{
			LOG(INFO) << "添加未登录的PlayerGateSession_World,SessionId=" << msg转发.gateClientSessionId;
			auto pair = g_mapPlayerGateSession.insert({ msg转发.gateClientSessionId,PlayerGateSession_World(*this,msg转发.gateClientSessionId) });
			iter = pair.first;
		}

		auto& refPlayerGateSession = iter->second;
		refPlayerGateSession.RecvMsg(msg.id, obj);
	}
	break;
	}
}
template<> std::deque<MsgGate转发>& WorldSessionFromGate::GetQueue() { return m_queueGate转发; }

void WorldSessionFromGate::OnInit(WorldSvrAcceptGate& refServer)
{
	refServer.m_Sessions.AddSession(&m_refSession, [this, &refServer]()
		{
			LOG(INFO) << "GateSvr已连上";
			m_pServer = &refServer;
			m_pSession = &m_refSession;

			//m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			//m_pServer->m_space.setEntity.insert(&m_entity);
		}, (uint64_t)this);
}

void WorldSessionFromGate::OnDestroy()
{
	//m_entity.OnDestroy();
	//m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
	//	{
	//		m_pServer->m_space.setEntity.erase(&m_entity);
	//	});

	//m_pServer = nullptr;//不用加锁
}