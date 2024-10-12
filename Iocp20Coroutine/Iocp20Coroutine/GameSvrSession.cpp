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
/// �����̣߳����߳�
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
int GameSvrSession::OnRecv(Session&, const void* buf, const int len)
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
	const auto msg = MsgHead::GetMsgId(obj);
	//++m_snRecv;
	//assert(m_snRecv == msg.sn);
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msg.id)
	{
	case MsgId::Gateת��:m_MsgQueue.PushMsg<MsgGateת��>(*this, obj); break;
		//case MsgId::GateAddSession:m_MsgQueue.PushMsg<MsgGateAddSession>(*this, obj); break;
		//case MsgId::GateDeleteSession:m_MsgQueue.PushMsg<MsgGateDeleteSession>(*this, obj); break;
	default:
		LOG(ERROR) << "û����msgId:" << msg.id;
		assert(false);
		break;
	}
}

//�����̣߳����߳�
void GameSvrSession::OnInit(GameSvr& server)
{
	server.m_Sessions.AddSession(&m_refSession, [this, &server]()
		{
			m_pServer = &server;
			//m_pWsSession = &refWsSession;
		}, (uint64_t)this);
}

//�����̣߳����߳�
void GameSvrSession::OnDestroy()
{
	for (auto& pair : m_mapPlayerGateSession)
	{
		pair.second.OnDestroy();
	}
	m_pServer = nullptr;//���ü���
}

//���̣߳����߳�
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
		case MsgId::Invalid_0:return true;//û����Ϣ�ɴ���
		case MsgId::Gateת��:return this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &GameSvrSession::OnRecv); break;
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
template<> std::deque<MsgGateת��>& GameSvrSession::GetQueue() { return m_queueGateת��; }
//template<> std::deque<MsgGateAddSession>& GameSvrSession::GetQueue() { return m_queueGateAddSession; }
//template<> std::deque<MsgGateDeleteSession>& GameSvrSession::GetQueue() { return m_queueGateDeleteSession; }


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
	const auto msg = MsgHead::GetMsgId(obj);
	LOG(INFO) << obj;

	switch (msg.id)
	{
	case MsgId::GateAddSession:OnRecv(obj.as<MsgGateAddSession>(), msgת��.gateClientSessionId); break;
	case MsgId::GateDeleteSession:OnRecv(obj.as<MsgGateDeleteSession>(), msgת��.gateClientSessionId); break;
	default:
	{
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

	//pair.first->second.EnterSpace(m_pServer->m_Space����ˢ��);
	pair.first->second.EnterSpace(pair.first->second.m_Space���˾���, msg.nickName);
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
	LOG(INFO) << "��ɾ��,idGateClientSession=" << idGateClientSession;
}
