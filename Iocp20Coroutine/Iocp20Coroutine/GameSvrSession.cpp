#include "StdAfx.h"
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
	const auto&& [bufPack, lenPack] = Iocp::OnRecv2(buf, len);
	if (lenPack > 0 && nullptr != bufPack)
	{
		OnRecvPack(bufPack, lenPack);
	}

	return lenPack;
}

void GameSvrSession::OnRecvPack(const void* buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
	LOG(INFO) << obj;
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msgId)
	{
	case MsgId::Gateת��:m_MsgQueue.PushMsg<MsgGateת��>(*this, obj); break;
	default:
		LOG(ERROR) << "û����msgId:" << msgId;
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
		});
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
	for (auto& pair : m_mapPlayerGateSession)
	{
		pair.second.Process();
	}
}
template<> std::deque<MsgGateת��>& GameSvrSession::GetQueue() { return m_queueGateת��; }


void GameSvrSession::OnRecv(const MsgGateת��& msg)
{
	if (msg.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}

	msgpack::object_handle oh = msgpack::unpack((const char*)&msg.vecByte[0], msg.vecByte.size());//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
	LOG(INFO) << obj;

	auto iter = m_mapPlayerGateSession.find(msg.gateClientSessionId);
	if (m_mapPlayerGateSession.end() == iter)
	{
		auto pair = m_mapPlayerGateSession.insert({ msg.gateClientSessionId, PlayerGateSession(*this) });
		if (!pair.second)
		{
			LOG(ERROR) << "ERR";
			assert(false);
			return;
		}
		iter = pair.first;
	}
	if (m_mapPlayerGateSession.end() == iter)
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}
	auto& refPlayerGateSession = iter->second;
	refPlayerGateSession.RecvMsg(msgId, obj);
}