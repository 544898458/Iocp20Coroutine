#include "pch.h"
#include "WorldSessionFromGame.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "WorldSvrAcceptGame.h"
//#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "../CoRoutine/CoDb.h"
#include "PlayerGateSession_World.h"
#include "DbPlayer.h"
#include "../CoRoutine/CoLock.h"

template class Iocp::SessionSocketCompletionKey<WorldSessionFromGame>;
template class MsgQueueMsgPack<WorldSessionFromGame>;
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
int WorldSessionFromGame::OnRecv(CompeletionKeySession&, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &WorldSessionFromGame::OnRecvPack);
}

bool WorldSessionFromGame::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		switch (msgId)
		{
		case MsgId::Invalid_0://û����Ϣ�ɴ���
			return true;
			//case MsgId::Login:	this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &WorldSession::OnRecv); break;
		//case MsgId::Say:	return this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &WorldSessionFromGame::OnRecv); break;
		//case MsgId::ChangeMoney:	return this->m_MsgQueue.OnRecv(this->m_queueConsumeMoney, *this, &WorldSessionFromGame::OnRecv); break;
		case MsgId::Gateת��:	return this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &WorldSessionFromGame::OnRecv); break;
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
/// �����̣߳����߳�
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void WorldSessionFromGame::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
		/*case MsgId::Say:m_MsgQueue.PushMsg<MsgSay>(*this, obj); break;
		case MsgId::ChangeMoney:m_MsgQueue.PushMsg<MsgChangeMoney>(*this, obj); break;
		*/case MsgId::Gateת��:m_MsgQueue.PushMsg<MsgGateת��>(*this, obj); break;
		default:
			LOG(WARNING) << "û����GameSvr��������Ϣ:" << msg.id;
			assert(false);
			break;
	}
}

void WorldSessionFromGame::OnRecv(const MsgSay& msg)
{
	LOG(INFO) << "GameSvr��������" << StrConv::Utf8ToGbk(msg.content);
	this->m_pServer->m_Sessions.Broadcast(msg);

}

void WorldSessionFromGame::OnRecv(const MsgGateת��& msgת��)
{
	//�����ظ����룬Ҫ�ϲ�

	if (msgת��.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}

	msgpack::object_handle oh = msgpack::unpack((const char*)&msgת��.vecByte[0], msgת��.vecByte.size());//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);

	switch (msg.id)
	{
		//case MsgId::Login:	OnRecv(obj.as<MsgLoginResponce>());	break;
		//case MsgId::GateDeleteSession:	pSession->Session.m_Session.OnRecvWorldSvr(obj.as<MsgGateDeleteSession>());	break;
			//case MsgId::Login:	this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &WorldSession::OnRecv); break;
	case MsgId::Say:	return OnRecv(obj.as<MsgSay>()); break;
	case MsgId::ChangeMoney:	return OnRecv(obj.as<MsgChangeMoney>(), msgת��.gateClientSessionId); break;
	}

}

extern CoDb<DbPlayer> g_CoDbPlayer;
/// <summary>
/// ע��Э����������ֲ����������ò�����Ҫȷ���첽��ֲ������Դ��ڣ�����Ҫ������
/// �˴��������̣߳����̣߳�ִ�У�����Э�̶������߳�ִ��
/// </summary>
/// <param name="msg"></param>
/// <returns></returns>
CoTask<int> WorldSessionFromGame::CoChangeMoney(const MsgChangeMoney msg, const std::string& strNickName)
{
	static FunCancel funCancel;
	CoLock lock;
	co_await lock.Lock(strNickName, funCancel);
	auto& refDb = *co_await DbPlayer::CoGet�������ؿ�(strNickName);
	MsgChangeMoneyResponce msgResponce;//{ .msg = {.rpcSnId = msg.msg.rpcSnId} };
	msgResponce.msg.rpcSnId = msg.msg.rpcSnId;
	assert(0 <= refDb.money);
	LOG(INFO) << "msg.addMoney=" << msg.addMoney << ",refDb.money=" << refDb.money;

	if (msg.addMoney)
	{
		if (std::numeric_limits< std::decay<decltype(refDb.money)>::type>::max() - refDb.money < msg.changeMoney)
		{
			msgResponce.error = -1;
		}
		else
		{
			refDb.money += msg.changeMoney;
		}
	}
	else
	{
		if (refDb.money < msg.changeMoney)
		{
			msgResponce.error = -2;
		}
		else
		{
			refDb.money -= msg.changeMoney;
		}
	}
	static FunCancel fun;
	co_await g_CoDbPlayer.CoSave(refDb, fun);

	msgResponce.finalMoney = refDb.money;
	this->Send(msgResponce);
	co_return 0;
}
//CoTask<int> WorldSessionFromGame::CoLogin(const MsgLogin msg, FunCancel& funCancel)
//{
//	co_return 0;
//}
void WorldSessionFromGame::OnRecv(const MsgChangeMoney& msg, const uint64_t idGateSessionId)
{
	//if (!m_coChangeMoney.Finished())//ĳЩ����Ӧ�ô����Ŷ�
	//{
	//	MsgChangeMoneyResponce msgResponce = { .error = 1 };
	//	msgResponce.msg.rpcSnId = msg.msg.rpcSnId;
	//	this->Send(msgResponce);
	//	return;
	//}
	extern std::map<uint64_t, PlayerGateSession_World> g_mapPlayerGateSession;
	const auto itFind = g_mapPlayerGateSession.find(idGateSessionId);
	assert(itFind != g_mapPlayerGateSession.end());
	if (itFind == g_mapPlayerGateSession.end())
	{
		return;
	}

	CoChangeMoney(msg,itFind->second.NickName()).RunNew();
}
//
//void WorldSessionFromGame::OnRecv(const MsgLogin& msg)
//{
//	if (!m_coLogin.Finished())
//	{
//		this->Send<MsgLoginResponce>({ .msg = {.rpcSnId = msg.msg.rpcSnId }, .result = MsgLoginResponce::Busy });
//		return;
//	}
//	m_coLogin = CoLogin(msg,m_funCancelLogin);
//	m_coChangeMoney.Run();
//}

//template<> std::deque<MsgSay>& WorldSessionFromGame::GetQueue() { return m_queueSay; }
//template<> std::deque<MsgChangeMoney>& WorldSessionFromGame::GetQueue() { return m_queueConsumeMoney; }
template<> std::deque<MsgGateת��>& WorldSessionFromGame::GetQueue() { return m_queueGateת��; }

void WorldSessionFromGame::OnInit(CompeletionKeySession& refSession, WorldSvrAcceptGame& refServer)
{
	refServer.m_Sessions.AddSession(&refSession, [this, &refSession, &refServer]()
		{
			LOG(INFO) << "GameSvr������";
			m_pServer = &refServer;
			m_pSession = &refSession;

			//m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			//m_pServer->m_space.setEntity.insert(&m_entity);
		}, (uint64_t)this);
}

void WorldSessionFromGame::OnDestroy()
{
	//m_entity.OnDestroy();
	//m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
	//	{
	//		m_pServer->m_space.setEntity.erase(&m_entity);
	//	});

	//m_pServer = nullptr;//���ü���
}