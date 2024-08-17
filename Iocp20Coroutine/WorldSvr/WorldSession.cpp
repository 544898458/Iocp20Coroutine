#include "pch.h"
#include "WorldSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "WorldServer.h"
//#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

template Iocp::SessionSocketCompletionKey<WorldSession>;
//template class WebSocketSession<WorldSession>;
//template void WebSocketSession<WorldSession>::OnInit<WorldServer>(Iocp::SessionSocketCompletionKey<WebSocketSession<WorldSession> >& refSession, WorldServer& server);
//template class WebSocketEndpoint<WorldSession, Iocp::SessionSocketCompletionKey<WebSocketSession<WorldSession> > >;

/// <summary>
/// 这是网络线程
/// </summary>
/// <param name=""></param>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <returns></returns>
int WorldSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
{
	const void* bufPack(nullptr);
	int lenPack(0);
	std::tie(bufPack, lenPack) = Iocp::OnRecv2(buf, len);
	if (lenPack > 0 && nullptr != bufPack)
	{
		OnRecvPack(bufPack, lenPack);
	}

	return lenPack;
}

void WorldSession::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//没判断越界，要加try
	LOG(INFO) << obj;

	switch (msgId)
	{
	case MsgId::Say:
		{
			const auto msg = obj.as<MsgSay>();
			LOG(INFO) << "GameSvr发来聊天" << StrConv::Utf8ToGbk(msg.content);
			this->m_pServer->m_Sessions.Broadcast(msg);
		}
		break;
	case MsgId::ConsumeMoney:
		{
		const auto msg = obj.as<MsgConsumeMoney>();
		LOG(INFO) << "GameSvr请求扣钱" << msg.consumeMoney;
		this->Send<MsgConsumeMoneyResponce>({.rpcSnId=msg.rpcSnId});
		}
		break;
	default:
		LOG(WARNING) << "没处理GameSvr发来的消息:" << msgId;
		break;
	}
}
void WorldSession::OnInit(CompeletionKeySession& refSession, WorldServer& refServer)
{
	refServer.m_Sessions.AddSession(&refSession, [this, &refSession, &refServer]()
		{
			LOG(INFO) << "GameSvr已连上";
			m_pServer = &refServer;
			m_pSession = &refSession;

			//m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			//m_pServer->m_space.setEntity.insert(&m_entity);
		});
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