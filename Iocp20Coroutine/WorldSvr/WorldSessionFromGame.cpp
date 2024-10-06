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

void WorldSessionFromGame::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//û����Ϣ�ɴ���
			break;
		switch (msgId)
		{
		case MsgId::Invalid_0://û����Ϣ�ɴ���
			return;
		//case MsgId::Login:	this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &WorldSession::OnRecv); break;
		case MsgId::Say:	this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &WorldSessionFromGame::OnRecv); break;
		case MsgId::ConsumeMoney:	this->m_MsgQueue.OnRecv(this->m_queueConsumeMoney, *this, &WorldSessionFromGame::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}
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
	case MsgId::Say:m_MsgQueue.PushMsg<MsgSay>(*this, obj); break;
	case MsgId::ConsumeMoney:m_MsgQueue.PushMsg<MsgChangeMoney>(*this, obj); break;
	default:
		LOG(WARNING) << "û����GameSvr��������Ϣ:" << msg.id;
		break;
	}
}

void WorldSessionFromGame::OnRecv(const MsgSay& msg)
{
	LOG(INFO) << "GameSvr��������" << StrConv::Utf8ToGbk(msg.content);
	this->m_pServer->m_Sessions.Broadcast(msg);

}

extern CoDb<DbPlayer> g_CoDbPlayer;
/// <summary>
/// ע��Э����������ֲ����������ò�����Ҫȷ���첽��ֲ������Դ��ڣ�����Ҫ������
/// �˴��������̣߳����̣߳�ִ�У�����Э�̶������߳�ִ��
/// </summary>
/// <param name="msg"></param>
/// <returns></returns>
CoTask<int> WorldSessionFromGame::CoChangeMoney(const MsgChangeMoney msg)
{
	static FunCancel funCancel;
	CoLock lock;
	co_await lock.Lock(msg.nickName, funCancel);
	auto& refDb = * co_await DbPlayer::CoGet�������ؿ�(msg.nickName);
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
CoTask<int> WorldSessionFromGame::CoLogin(const MsgLogin msg, FunCancel &funCancel)
{
	co_return 0;
}
void WorldSessionFromGame::OnRecv(const MsgChangeMoney& msg)
{
	//if (!m_coChangeMoney.Finished())//ĳЩ����Ӧ�ô����Ŷ�
	//{
	//	MsgChangeMoneyResponce msgResponce = { .error = 1 };
	//	msgResponce.msg.rpcSnId = msg.msg.rpcSnId;
	//	this->Send(msgResponce);
	//	return;
	//}
	CoChangeMoney(msg).RunNew();
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

template<> std::deque<MsgSay>& WorldSessionFromGame::GetQueue() { return m_queueSay; }
template<> std::deque<MsgChangeMoney>& WorldSessionFromGame::GetQueue() { return m_queueConsumeMoney; }

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