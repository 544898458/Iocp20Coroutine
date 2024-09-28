#include "pch.h"
#include "WorldSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "WorldServer.h"
//#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "../CoRoutine/CoDb.h"
#include "../CoRoutine/CoDbTemplate.h"
#include "PlayerGateSession.h"

template class Iocp::SessionSocketCompletionKey<WorldSession>;
template class MsgQueueMsgPack<WorldSession>;
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
int WorldSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &WorldSession::OnRecvPack);
}

void WorldSession::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//没有消息可处理
			break;
		switch (msgId)
		{
		case MsgId::Invalid_0://没有消息可处理
			return;
		//case MsgId::Login:	this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &WorldSession::OnRecv); break;
		case MsgId::Gate转发:	this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &WorldSession::OnRecv); break;
		case MsgId::Say:	this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &WorldSession::OnRecv); break;
		case MsgId::ConsumeMoney:	this->m_MsgQueue.OnRecv(this->m_queueConsumeMoney, *this, &WorldSession::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}
}

/// <summary>
/// 网络线程，多线程
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void WorldSession::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
	//case MsgId::Login:m_MsgQueue.PushMsg<MsgLogin>(*this, obj); break;
	case MsgId::Say:m_MsgQueue.PushMsg<MsgSay>(*this, obj); break;
	case MsgId::ConsumeMoney:m_MsgQueue.PushMsg<MsgChangeMoney>(*this, obj); break;
	default:
		LOG(WARNING) << "没处理GameSvr发来的消息:" << msg.id;
		break;
	}
}

void WorldSession::OnRecv(const MsgSay& msg)
{
	LOG(INFO) << "GameSvr发来聊天" << StrConv::Utf8ToGbk(msg.content);
	this->m_pServer->m_Sessions.Broadcast(msg);

}
std::map<uint64_t,PlayerGateSession> g_mapPlayerGateSession;
void WorldSession::OnRecv(const MsgGate转发& msg转发)
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
	if (g_mapPlayerGateSession.end() == iter)
	{
		auto pair = g_mapPlayerGateSession.insert({ msg转发.gateClientSessionId,PlayerGateSession(*this,msg转发.gateClientSessionId)});
		iter = pair.first;
	}

	auto& refPlayerGateSession = iter->second;
	
	refPlayerGateSession.RecvMsg(msg.id, obj);
}

extern CoDb<DbTest> g_TestSave;
/// <summary>
/// 注意协程里如果传局部变量的引用参数，要确保异步后局部变量仍存在，否则不要传引用
/// 此代码在主线程（单线程）执行，所有协程都在主线程执行
/// </summary>
/// <param name="msg"></param>
/// <returns></returns>
CoTask<int> WorldSession::Save(const MsgChangeMoney msg)
{
	static FunCancel fun;
	//LOG(INFO) << "GameSvr请求扣钱" << msg.changeMoney;
	if (m_mapMoney.find(msg.nickName) == m_mapMoney.end())
	{
		DbTest loadDb = co_await g_TestSave.Load(msg.nickName, fun);
		m_mapMoney.insert({ msg.nickName, loadDb });
	}
	auto& refDb = m_mapMoney[msg.nickName];
	MsgChangeMoneyResponce msgResponce = { .msg = {.rpcSnId = msg.msg.rpcSnId} };
	assert(0 <= refDb.money);
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
	co_await g_TestSave.Save(refDb, fun);

	msgResponce.finalMoney = refDb.money;
	this->Send(msgResponce);
	co_return 0;
}
CoTask<int> WorldSession::CoLogin(const MsgLogin msg, FunCancel &funCancel)
{
	co_return 0;
}
void WorldSession::OnRecv(const MsgChangeMoney& msg)
{
	if (!m_coChangeMoney.Finished())
	{
		this->Send<MsgChangeMoneyResponce>({ .msg = {.rpcSnId = msg.msg.rpcSnId }, .error = 1 });
		return;
	}
	m_coChangeMoney = Save(msg);
	m_coChangeMoney.Run();
}

void WorldSession::OnRecv(const MsgLogin& msg)
{
	if (!m_coLogin.Finished())
	{
		this->Send<MsgLoginResponce>({ .msg = {.rpcSnId = msg.msg.rpcSnId }, .error = 1 });
		return;
	}
	m_coLogin = CoLogin(msg,m_funCancelLogin);
	m_coChangeMoney.Run();
}

template<> std::deque<MsgSay>& WorldSession::GetQueue() { return m_queueSay; }
template<> std::deque<MsgChangeMoney>& WorldSession::GetQueue() { return m_queueConsumeMoney; }
//template<> std::deque<MsgLogin>& WorldSession::GetQueue() { return m_queueLogin; }

void WorldSession::OnInit(CompeletionKeySession& refSession, WorldServer& refServer)
{
	refServer.m_Sessions.AddSession(&refSession, [this, &refSession, &refServer]()
		{
			LOG(INFO) << "GameSvr已连上";
			m_pServer = &refServer;
			m_pSession = &refSession;

			//m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			//m_pServer->m_space.setEntity.insert(&m_entity);
		}, (uint64_t)this);
}

void WorldSession::OnDestroy()
{
	//m_entity.OnDestroy();
	//m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
	//	{
	//		m_pServer->m_space.setEntity.erase(&m_entity);
	//	});

	//m_pServer = nullptr;//不用加锁
}