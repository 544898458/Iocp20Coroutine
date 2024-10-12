#include "pch.h"
#include <glog/logging.h>

//#include "IocpNetwork/ListenSocketCompletionKey.cpp"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
//#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "GameSvrSession.h"

//#include <iostream>
#include <cassert>

#include <codecvt>
//#include "MsgQueue.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "GameSvr.h"
#include "AiCo.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoEvent.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "AttackComponent.h"
#include "../IocpNetwork/MsgPack.h"
#include "PlayerGateSession_Game.h"
//template<MySession>
//std::set<Iocp::SessionSocketCompletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;
template Iocp::SessionSocketCompletionKey<GameSvrSession>;
//template class WebSocketSession<GameSvrSession>;
//template void WebSocketSession<GameSvrSession>::OnInit<GameSvr>(Iocp::SessionSocketCompletionKey<WebSocketSession<GameSvrSession> >& refSession, GameSvr& server);
//template class WebSocketEndpoint<GameSvrSession, Iocp::SessionSocketCompletionKey<WebSocketSession<GameSvrSession> > >;

/// <summary>
/// 网络线程，多线程
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
int GameSvrSession::OnRecv(Session&, const void* buf, const int len)
{
	return Iocp::OnRecv3(buf, len, *this, &GameSvrSession::OnRecvPack);
}

/// <summary>
/// 这是网络线程，多线程
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void GameSvrSession::OnRecvPack(const void* buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	LOG(INFO) << obj;
	const auto msg = MsgHead::GetMsgId(obj);
	//++m_snRecv;
	//assert(m_snRecv == msg.sn);
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msg.id)
	{
	case MsgId::Gate转发:m_MsgQueue.PushMsg<MsgGate转发>(*this, obj); break;
		//case MsgId::GateAddSession:m_MsgQueue.PushMsg<MsgGateAddSession>(*this, obj); break;
		//case MsgId::GateDeleteSession:m_MsgQueue.PushMsg<MsgGateDeleteSession>(*this, obj); break;
	default:
		LOG(ERROR) << "没处理msgId:" << msg.id;
		assert(false);
		break;
	}
}

//网络线程，多线程
void GameSvrSession::OnInit(GameSvr& server)
{
	server.m_Sessions.AddSession(&m_refSession, [this, &server]()
		{
			m_pServer = &server;
			//m_pWsSession = &refWsSession;
		}, (uint64_t)this);
}

//网络线程，多线程
void GameSvrSession::OnDestroy()
{
	for (auto& pair : m_mapPlayerGateSession)
	{
		pair.second.OnDestroy();
	}
	m_pServer = nullptr;//不用加锁
}

//主线程，单线程
bool GameSvrSession::Process()
{
	ProcessMsg();

	for (auto& pair : m_mapPlayerGateSession)
	{
		pair.second.Process();
	}

	return true;
}
bool GameSvrSession::ProcessMsg()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		switch (msgId)
		{
		case MsgId::Invalid_0:return true;//没有消息可处理
		case MsgId::Gate转发:return this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &GameSvrSession::OnRecv); break;
			//case MsgId::GateAddSession:return this->m_MsgQueue.OnRecv(this->m_queueGateAddSession, *this, &GameSvrSession::OnRecv); break;
			//case MsgId::GateDeleteSession:return this->m_MsgQueue.OnRecv(this->m_queueGateDeleteSession, *this, &GameSvrSession::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			return false;
			break;
		}
		assert(false);
	}
}
template<> std::deque<MsgGate转发>& GameSvrSession::GetQueue() { return m_queueGate转发; }
//template<> std::deque<MsgGateAddSession>& GameSvrSession::GetQueue() { return m_queueGateAddSession; }
//template<> std::deque<MsgGateDeleteSession>& GameSvrSession::GetQueue() { return m_queueGateDeleteSession; }


void GameSvrSession::OnRecv(const MsgGate转发& msg转发)
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

	switch (msg.id)
	{
	case MsgId::GateAddSession:OnRecv(obj.as<MsgGateAddSession>(), msg转发.gateClientSessionId); break;
	case MsgId::GateDeleteSession:OnRecv(obj.as<MsgGateDeleteSession>(), msg转发.gateClientSessionId); break;
	default:
	{
		auto iter = m_mapPlayerGateSession.find(msg转发.gateClientSessionId);
		if (m_mapPlayerGateSession.end() == iter)
		{
			LOG(ERROR) << "ERR";
			assert(false);
			return;
		}

		auto& refPlayerGateSession = iter->second;

		refPlayerGateSession.RecvMsg(msg.id, obj);
	}
	break;
	}

}

void GameSvrSession::OnRecv(const MsgGateAddSession& msg, const uint64_t idGateClientSession)
{
	auto iterOld = m_mapPlayerGateSession.find(idGateClientSession);
	if (m_mapPlayerGateSession.end() != iterOld)
	{
		LOG(ERROR) << "err";
		assert(false);
		m_mapPlayerGateSession.erase(iterOld);
	}

	auto pair = m_mapPlayerGateSession.insert({ idGateClientSession, PlayerGateSession_Game(*this,idGateClientSession) });
	if (!pair.second)
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}

	//pair.first->second.EnterSpace(m_pServer->m_Space无限刷怪);
	pair.first->second.EnterSpace(pair.first->second.m_Space单人剧情, msg.nickName);
}

void GameSvrSession::OnRecv(const MsgGateDeleteSession& msg, const uint64_t idGateClientSession)
{
	auto iterOld = m_mapPlayerGateSession.find(idGateClientSession);
	if (m_mapPlayerGateSession.end() == iterOld)
	{
		LOG(ERROR) << "err";
		assert(false);
		return;
	}

	iterOld->second.OnDestroy();
	m_mapPlayerGateSession.erase(iterOld);
	LOG(INFO) << "已删除,idGateClientSession=" << idGateClientSession;
}
