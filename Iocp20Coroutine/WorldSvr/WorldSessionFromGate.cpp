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
/// �����̣߳����߳�
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
		case MsgId::MsgId_Invalid_0://û����Ϣ�ɴ���
			return true;
			//case MsgId::Login:	this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &WorldSession::OnRecv); break;
		case MsgId::Gateת��:	return this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &WorldSessionFromGate::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			_ASSERT(false);
			return false;
			break;
		}
	}

	_ASSERT(false);
	return false;
}

/// <summary>
/// �����̣߳����߳�
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void WorldSessionFromGate::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
	case MsgId::Gateת��:m_MsgQueue.PushMsg<MsgGateת��>(*this, obj); break;
	default:
		LOG(WARNING) << "û����GameSvr��������Ϣ:" << msg.id;
		break;
	}
}

template<typename T> void BroadcastToGate(const T& refMsg);

std::map<uint64_t, PlayerGateSession_World> g_mapPlayerGateSession;
extern std::map<std::string, uint64_t> g_mapPlayerNickNameGateSessionId;

void BroadcastToGate��������()
{
	Msg�������� msg = { .u16���� = (uint16_t)g_mapPlayerNickNameGateSessionId.size() };
	for (auto [strNickName, v] : g_mapPlayerNickNameGateSessionId)
	{
		msg.vec���NickName.push_back(StrConv::GbkToUtf8(strNickName));
	}
	BroadcastToGate(msg);
}
void WorldSessionFromGate::OnRecv(const MsgGateת��& msgת��)
{
	if (msgת��.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		_ASSERT(false);
		return;
	}

	msgpack::object_handle oh = msgpack::unpack((const char*)&msgת��.vecByte[0], msgת��.vecByte.size());//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	LOG(INFO) << obj;

	auto iter = g_mapPlayerGateSession.find(msgת��.gateClientSessionId);
	switch (msg.id)
	{
	case MsgId::GateDeleteSession:
	{
		//_ASSERT(g_mapPlayerGateSession.end() != iter);
		if (g_mapPlayerGateSession.end() == iter)
		{
			LOG(WARNING) << "������Э��ĺڿ͹���";//����telnet��˿ڷ�����ģ��
			return;
		}
		auto& refPlayerGateSession = iter->second;

		{
			auto iterFind = g_mapPlayerNickNameGateSessionId.find(refPlayerGateSession.NickName());
			if (g_mapPlayerNickNameGateSessionId.end() == iterFind)
			{
				LOG(WARNING) << "��û��¼������ɾ��:" << refPlayerGateSession.NickName();
				return;
			}
			LOG(INFO) << refPlayerGateSession.NickName() << ",ɾ�����ֶ�Ӧ��SessionId:" << msgת��.gateClientSessionId;
			g_mapPlayerNickNameGateSessionId.erase(iterFind);
			void BroadcastToGate��������();
			BroadcastToGate��������();
		}

		g_mapPlayerGateSession.erase(iter);
		LOG(INFO) << "ɾ��PlayerGateSession_World����,Id=" << msgת��.gateClientSessionId;

	}
	break;
	default:
	{
		if (g_mapPlayerGateSession.end() == iter)
		{
			LOG(INFO) << "���δ��¼��PlayerGateSession_World,SessionId=" << msgת��.gateClientSessionId;
			auto pair = g_mapPlayerGateSession.insert({ msgת��.gateClientSessionId,PlayerGateSession_World(*this,msgת��.gateClientSessionId) });
			iter = pair.first;
		}

		auto& refPlayerGateSession = iter->second;
		refPlayerGateSession.RecvMsg(msg.id, obj);
	}
	break;
	}
}
template<> std::deque<MsgGateת��>& WorldSessionFromGate::GetQueue() { return m_queueGateת��; }

void WorldSessionFromGate::OnInit(WorldSvrAcceptGate& refServer)
{
	refServer.m_Sessions.AddSession(&m_refSession, [this, &refServer]()
		{
			LOG(INFO) << "GateSvr������";
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

	//m_pServer = nullptr;//���ü���
}