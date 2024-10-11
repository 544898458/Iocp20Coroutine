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
template void WebSocketSession<GateSession>::OnInit<GateServer>(GateServer& server);
//int GateSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
//{
//    return 0;
//}

void SendToGameSvrת��(const void* buf, const int len, uint64_t gateSessionId);// , uint32_t sn);
template<class T> void SendToWorldSvrת��(const T& refMsg, const uint64_t gateSessionId);// , uint32_t snSend);
template<class T> void SendToGameSvr(const T& refMsg, const uint64_t gateSessionId, uint32_t snSend);

void GateSession::OnRecvWsPack(const void* buf, const int len)
{
	try
	{
		msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
		msgpack::object obj = oh.get();
		const auto msg = MsgHead::GetMsgId(obj);
		//LOG(INFO) << obj;

		switch (msg.id)
		{
		case MsgId::Login:	m_MsgQueue.PushMsg<MsgLogin>(*this, obj); break;
		default:
			LOG(INFO) << "ת��GameSvr��Ϣ:" << msg.id;
			SendToGameSvrת��(buf, len, GetId());// , ++m_snSend);
			break;
		}
	}
	catch (const msgpack::unpack_error& error)
	{
		LOG(WARNING) << "MsgPack���ʧ��:" << error.what();
		assert(false);
	}

}
template<> std::deque<MsgLogin>& GateSession::GetQueue() { return m_queueLogin; }
void GateSession::OnRecv(const MsgLogin& msg)
{
	LOG(INFO) << "GameSvr������¼";
	if (!m_coLogin.Finished() || m_bLoginOk)
	{
		LOG(ERROR) << "m_coLogin.Finished=" << m_coLogin.Finished() << ",m_bLoginOk=" << m_bLoginOk;
		return;
	}
	m_coLogin = CoLogin(msg, m_funCancelLogin);
	m_coLogin.Run();
}

CoTask<int> GateSession::CoLogin(MsgLogin msg, FunCancel& funCancel)
{
	assert(!m_bLoginOk);
	{
		auto [stop, responce] = co_await CoRpc<MsgLoginResponce>::Send<MsgLogin>(msg, [this](const MsgLogin& msg) {SendToWorldSvrת��<MsgLogin>(msg, GetId()); }, funCancel);
		LOG(INFO) << "WorldSvr���ص�¼���stop=" << stop << ",error=" << responce.result;
		if (stop)
		{
			LOG(WARNING) << "Э��ֹͣ";
			co_return 0;
		}

		if (responce.result != MsgLoginResponce::OK)
		{
			LOG(WARNING) << "��¼ʧ��";
			co_return 0;
		}
		m_bLoginOk = true;

	}

	{
		//��¼GameSvr
		SendToGameSvr(MsgGateAddSession(), GetId(), ++m_snSendToGameSvr);
	}
	co_return 0;
}

void GateSession::OnDestroy()
{
	if (m_bClosed)
	{
		return;
	}
	m_bClosed = true;
	SendToGameSvr<MsgGateDeleteSession>({}, GetId(), ++m_snSendToGameSvr);
	SendToWorldSvrת��<MsgGateDeleteSession>({}, GetId());// , ++m_snSendToWorldSvr);
}

void GateSession::OnInit(GateServer& refServer)
{
	refServer.m_Sessions.AddSession(&m_refSession.m_refSession, [this, &refServer]()
		{
			LOG(INFO) << "��Ϸ�ͻ���������";
			//m_pServer = &refServer;
			//m_pSession = &refSession;
		}, GetId());

}

void GateSession::OnRecvWorldSvr(const MsgLoginResponce& msg)
{
	CoRpc<MsgLoginResponce>::OnRecvResponce(false, msg);
}

void GateSession::OnRecvWorldSvr(const MsgGateDeleteSession& msg)
{
	LOG(INFO) << "�����Ͽ���Ϸ�ͻ���Socket,Id=" << GetId(),
		m_refSession.m_refSession.CloseSocket();

	OnDestroy();
	MsgGateDeleteSessionResponce msgResponce;
	msgResponce.msg.rpcSnId = msg.msg.rpcSnId;
	SendToWorldSvrת��(msgResponce, GetId());
}

bool GateSession::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		switch (msgId)
		{
		case MsgId::Invalid_0://û����Ϣ�ɴ���
			return true;
		case MsgId::Login:return this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &GateSession::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			return false;
			break;
		}
		assert(false);
	}
	return true;
}
