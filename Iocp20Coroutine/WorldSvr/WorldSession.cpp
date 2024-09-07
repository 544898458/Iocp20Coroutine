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
		case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &WorldSession::OnRecv); break;
		case MsgId::ConsumeMoney:this->m_MsgQueue.OnRecv(this->m_queueConsumeMoney, *this, &WorldSession::OnRecv); break;
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
	const auto msgId = Msg::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msgId)
	{
	case MsgId::Say:m_MsgQueue.PushMsg<MsgSay>(*this, obj); break;
	case MsgId::ConsumeMoney:m_MsgQueue.PushMsg<MsgChangeMoney>(*this, obj); break;
	default:
		LOG(WARNING) << "没处理GameSvr发来的消息:" << msgId;
		break;
	}
}

void WorldSession::OnRecv(const MsgSay& msg)
{
	LOG(INFO) << "GameSvr发来聊天" << StrConv::Utf8ToGbk(msg.content);
	this->m_pServer->m_Sessions.Broadcast(msg);

}

void WorldSession::OnRecv(const MsgChangeMoney& msg)
{
	//LOG(INFO) << "GameSvr请求扣钱" << msg.changeMoney;
	auto& refMoney = m_mapMoney[msg.nickName];
	MsgChangeMoneyResponce msgResponce = {.rpcSnId = msg.rpcSnId};
	assert(0 <= refMoney);
	if (msg.addMoney)
	{
		if (std::numeric_limits< std::decay<decltype(refMoney)>::type>::max() - refMoney < msg.changeMoney)
		{
			msgResponce.error = -1;
		}
		else
		{
			refMoney += msg.changeMoney;
		}
	}
	else 
	{
		if (refMoney < msg.changeMoney)
		{
			msgResponce.error = -2;
		}
		else
		{
			refMoney -= msg.changeMoney;
		}
	}
	msgResponce.finalMoney = refMoney;
	this->Send(msgResponce);
}

template<> std::deque<MsgSay>& WorldSession::GetQueue() { return m_queueSay; }
template<> std::deque<MsgChangeMoney>& WorldSession::GetQueue() { return m_queueConsumeMoney; }

void WorldSession::OnInit(CompeletionKeySession& refSession, WorldServer& refServer)
{
	refServer.m_Sessions.AddSession(&refSession, [this, &refSession, &refServer]()
		{
			LOG(INFO) << "GameSvr已连上";
			m_pServer = &refServer;
			m_pSession = &refSession;

			//m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			//m_pServer->m_space.setEntity.insert(&m_entity);
		},(uint64_t)this);
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