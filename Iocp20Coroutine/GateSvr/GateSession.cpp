#include "pch.h"
#include "GateSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "GateServer.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "../CoRoutine/CoRpc.h"

template class Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession>;
template class MsgQueueMsgPack<GateSession::CompeletionKeySession>;
template class WebSocketSession<GateSession>;
template class WebSocketEndpoint<GateSession, Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession> >;
template void WebSocketSession<GateSession>::OnInit<GateServer>(Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession>& refSession, GateServer& server);
//int GateSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
//{
//    return 0;
//}

void SendToGameSvr(const void* buf, const int len, uint64_t gateSessionId);// , uint32_t sn);
void GateSession::OnRecvWsPack(const void* buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
	case MsgId::Login:	m_MsgQueue.PushMsg<MsgLogin>(*this, obj); break;
	default:
		LOG(WARNING) << "转发GameSvr消息:" << msg.id;
		SendToGameSvr(buf, len, (uint64_t)this);// , ++m_snSend);
		break;
	}
	
}
template<> std::deque<MsgLogin>& GateSession::GetQueue() { return m_queueLogin; }
void GateSession::OnRecv(const MsgLogin& msg)
{
	LOG(INFO) << "GameSvr发来登录" ;
	if (!m_coLogin.Finished()||m_bLoginOk)
	{
		LOG(ERROR) << "m_coLogin.Finished=" << m_coLogin.Finished() << ",m_bLoginOk=" << m_bLoginOk;
		return;
	}
	m_coLogin = CoLogin(msg, m_funCancelLogin);
	m_coLogin.Run();
}

template<class T>
void SendToWorldSvr(const T& refMsg, const uint64_t gateSessionId);// , uint32_t snSend);

CoTask<int> GateSession::CoLogin(MsgLogin msg, FunCancel &funCancel)
{
	LOG(INFO) << "GameSvr发来登录";
	auto responce = co_await CoRpc<MsgLoginResponce>::Send<MsgLogin>(msg, [this](const MsgLogin& msg) {SendToWorldSvr<MsgLogin>(msg, (uint64_t)this); }, funCancel);
	co_return 0;
}

template<class T>
void SendToGameSvr(const T& refMsg);// , uint32_t snSend);

void GateSession::OnDestroy()
{
	SendToGameSvr<MsgGateDeleteSession>({ .gateClientSessionId = (uint64_t)this });// , ++m_snSend);
}

void GateSession::OnInit(CompeletionKeySession& refSession, GateServer& refServer)
{
	refServer.m_Sessions.AddSession(refSession.m_pSession, [this, &refSession, &refServer]()
		{
			LOG(INFO) << "游戏客户端已连上";
			//m_pServer = &refServer;
			//m_pSession = &refSession;

			SendToGameSvr<MsgGateAddSession>({ .gateClientSessionId = (uint64_t)this });// , ++m_snSend);
		}, (uint64_t)this);

}

void GateSession::OnRecvWorldSvr(const MsgLoginResponce& msg)
{
	CoRpc<MsgLoginResponce>::OnRecvResponce(false, msg);
}

void GateSession::Process()
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
		case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &GateSession::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}
}
