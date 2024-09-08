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
/// �����̣߳����߳�
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
int GameSvrSession::OnRecv(WebSocketGameSession&, const void* buf, const int len)
{
	return Iocp::OnRecv3(buf, len, *this, &GameSvrSession::OnRecvPack);
}

/// <summary>
/// ���������̣߳����߳�
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void GameSvrSession::OnRecvPack(const void* buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	LOG(INFO) << obj;
	const auto msg = Msg::GetMsgId(obj);
	//++m_snRecv;
	//assert(m_snRecv == msg.sn);
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msg.id)
	{
	case MsgId::Gateת��:m_MsgQueue.PushMsg<MsgGateת��>(*this, obj); break;
	case MsgId::GateAddSession:m_MsgQueue.PushMsg<MsgGateAddSession>(*this, obj); break;
	case MsgId::GateDeleteSession:m_MsgQueue.PushMsg<MsgGateDeleteSession>(*this, obj); break;
	default:
		LOG(ERROR) << "û����msgId:" << msg.id;
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
	m_pServer = nullptr;//���ü���
}

//���̣߳����߳�
void GameSvrSession::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//û����Ϣ�ɴ���
			break;

		switch (msgId)
		{
		case MsgId::Gateת��:this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &GameSvrSession::OnRecv); break;
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
template<> std::deque<MsgGateת��>& GameSvrSession::GetQueue() { return m_queueGateת��; }
template<> std::deque<MsgGateAddSession>& GameSvrSession::GetQueue() { return m_queueGateAddSession; }
template<> std::deque<MsgGateDeleteSession>& GameSvrSession::GetQueue() { return m_queueGateDeleteSession; }


void GameSvrSession::OnRecv(const MsgGateת��& msgת��)
{
	if (msgת��.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}

	msgpack::object_handle oh = msgpack::unpack((const char*)&msgת��.vecByte[0], msgת��.vecByte.size());//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msg = Msg::GetMsgId(obj);
	LOG(INFO) << obj;

	auto iter = m_mapPlayerGateSession.find(msgת��.gateClientSessionId);
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
