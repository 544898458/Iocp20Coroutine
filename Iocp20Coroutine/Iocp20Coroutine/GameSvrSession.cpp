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
#include "MyServer.h"
#include "AiCo.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "AttackComponent.h"
#include "../IocpNetwork/MsgPack.h"
#include "PlayerGateSession.h"
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
int GameSvrSession::OnRecv(WebSocketGameSession&, const void* buf, const int len)
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
	const auto msg = Msg::GetMsgId(obj);
	//++m_snRecv;
	//assert(m_snRecv == msg.sn);
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msg.id)
	{
	case MsgId::Gate转发:m_MsgQueue.PushMsg<MsgGate转发>(*this, obj); break;
	case MsgId::GateAddSession:m_MsgQueue.PushMsg<MsgGateAddSession>(*this, obj); break;
	case MsgId::GateDeleteSession:m_MsgQueue.PushMsg<MsgGateDeleteSession>(*this, obj); break;
	default:
		LOG(ERROR) << "没处理msgId:" << msg.id;
		assert(false);
		break;
	}
}

void GameSvrSession::OnInit(WebSocketGameSession& refWsSession, GameSvr& server)
{
	server.m_Sessions.AddSession(&refWsSession, [this, &refWsSession, &server]()
		{
			m_pServer = &server;
			m_pWsSession = &refWsSession;
		}, (uint64_t)this);
}

void GameSvrSession::OnDestroy()
{
	for (auto& pair : m_mapPlayerGateSession)
	{
		pair.second.OnDestroy(m_pServer->m_space);
	}
	m_pServer = nullptr;//不用加锁
}

//主线程，单线程
void GameSvrSession::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//没有消息可处理
			break;

		switch (msgId)
		{
		case MsgId::Gate转发:this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &GameSvrSession::OnRecv); break;
		case MsgId::GateAddSession:this->m_MsgQueue.OnRecv(this->m_queueGateAddSession, *this, &GameSvrSession::OnRecv); break;
		case MsgId::GateDeleteSession:this->m_MsgQueue.OnRecv(this->m_queueGateDeleteSession, *this, &GameSvrSession::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}

	for (auto& pair : m_mapPlayerGateSession)
	{
		pair.second.Process();
	}
}
template<> std::deque<MsgGate转发>& GameSvrSession::GetQueue() { return m_queueGate转发; }
template<> std::deque<MsgGateAddSession>& GameSvrSession::GetQueue() { return m_queueGateAddSession; }
template<> std::deque<MsgGateDeleteSession>& GameSvrSession::GetQueue() { return m_queueGateDeleteSession; }


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
	const auto msg = Msg::GetMsgId(obj);
	LOG(INFO) << obj;

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

void GameSvrSession::OnRecv(const MsgGateAddSession& msg)
{
	auto iterOld = m_mapPlayerGateSession.find(msg.gateClientSessionId);
	if (m_mapPlayerGateSession.end() != iterOld)
	{
		LOG(ERROR) << "err";
		assert(false);
		m_mapPlayerGateSession.erase(iterOld);
	}

	auto pair = m_mapPlayerGateSession.insert({ msg.gateClientSessionId, PlayerGateSession(*this,msg.gateClientSessionId) });
	if (!pair.second)
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}

}

void GameSvrSession::OnRecv(const MsgGateDeleteSession& msg)
{
	auto iterOld = m_mapPlayerGateSession.find(msg.gateClientSessionId);
	if (m_mapPlayerGateSession.end() == iterOld)
	{
		LOG(ERROR) << "err";
		assert(false);
		return;
	}

	iterOld->second.OnDestroy(m_pServer->m_space);
	m_mapPlayerGateSession.erase(iterOld);
}
