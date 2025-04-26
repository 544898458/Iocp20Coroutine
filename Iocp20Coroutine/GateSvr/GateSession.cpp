#include "pch.h"
#include "GateSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../GameSvr/MyMsgQueue.h"
#include "GateServer.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTask.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"
#include "../�������ļ�/Try��Ini���ػ���ר��.h"

template class Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession>;
template class MsgQueueMsgPack<GateSession::CompeletionKeySession>;
template class WebSocketSession<GateSession>;
template class WebSocketEndpoint<GateSession, GateSession::CompeletionKeySession>;
template void WebSocketSession<GateSession>::OnInit<GateServer>(GateServer& server);
//int GateSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
//{
//    return 0;
//}

//void SendToGameSvrת��(const void* buf, const int len, uint64_t gateSessionId);// , uint32_t sn);
template<class T> void SendToWorldSvrת��(const T& refMsg, const uint64_t gateSessionId);// , uint32_t snSend);
template<class T> void SendToGameSvr(const T& refMsg, const uint64_t gateSessionId, uint32_t snSend);
template<> void SendToGameSvr(const MsgGateת��& refMsg, const uint64_t gateSessionId, uint32_t snSend);

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
		case MsgId::Gateת��:
			LOG(ERROR) << "�ڿͷ���ת��Gateת��:" << msg.id;
			break;
		default:
			//LOG(INFO) << "ת��GameSvr��Ϣ:" << msg.id;
			//SendToGameSvrת��(buf, len, GetId());// , ++m_snSend);
			m_MsgQueue.PushMsg<MsgGateת��>(*this, MsgGateת��(buf, len, GetId(), 0)); break;
			break;
		}
	}
	catch (const msgpack::unpack_error& error)
	{
		LOG(WARNING) << "unpack_error,MsgPack���ʧ��:" << error.what();
		//_ASSERT(false);
	}
	catch (const msgpack::type_error& error)
	{
		LOG(WARNING) << "type_error,MsgPack���ʧ��:" << error.what();
		//_ASSERT(false);
	}

}
template<> std::deque<MsgLogin>& GateSession::GetQueue() { return m_queueLogin; }
template<> std::deque<MsgGateת��>& GateSession::GetQueue() { return m_queueMsgGateת��; }
void GateSession::OnRecv(const MsgLogin& msg)
{
	if (msg.name.empty())
	{
		m_refSession.m_refSession.CloseSocket();
	}
	LOG(INFO) << "��ҷ�����¼," << StrConv::Utf8ToGbk(msg.name);
	if (!m_coLogin.Finished() || m_bLoginOk)
	{
		LOG(ERROR) << "m_coLogin.Finished=" << m_coLogin.Finished() << ",m_bLoginOk=" << m_bLoginOk;
		return;
	}
	m_coLogin = CoLogin(msg, m_funCancelLogin);
	m_coLogin.Run();
}

void GateSession::OnRecv(const MsgGateת��& msg)
{
	SendToGameSvr(msg, GetId(), ++m_snSendToGameSvr);
}

template<class T>
void SendToGateClient(const T& refMsg, uint64_t gateSessionId);
CoTask<int> GateSession::CoLogin(MsgLogin msg, FunCancel& funCancel)
{
	KeepCancel kc(funCancel);

	_ASSERT(!m_bLoginOk);
	{
		using namespace std;

		uint32_t �汾�ſɽ���� = 0;
		uint32_t �汾�ſɽ���� = 0;
		Try��Ini���ػ���ר��(�汾�ſɽ����, "GateSvr", "VerMax");
		Try��Ini���ػ���ר��(�汾�ſɽ����, "GateSvr", "VerMin");
		if (�汾�ſɽ���� < msg.u32�汾�� || �汾�ſɽ���� > msg.u32�汾��)
		{
			LOG(WARNING) << "[" << �汾�ſɽ���� << "," << �汾�ſɽ���� << "],�汾:" << msg.u32�汾��;
			SendToGateClient<MsgLoginResponce>({ .result = MsgLoginResponce::�ͻ��˰汾��ƥ��, .str��ʾ = StrConv::GbkToUtf8("�汾��ƥ��") }, (uint64_t)this);
			if (co_await CoTimer::Wait(1s, funCancel))
				co_return 0;

			m_refSession.m_refSession.CloseSocket();
			co_return 0;
		}
		auto [stop, responce] = co_await CoRpc<MsgLoginResponce>::Send<MsgLogin>(msg, [this](const MsgLogin& msg) {SendToWorldSvrת��<MsgLogin>(msg, GetId()); }, funCancel);
		LOG(INFO) << "WorldSvr���ص�¼���stop=" << stop << ",error=" << responce.result;
		if (stop)
		{
			LOG(WARNING) << "Э��ֹͣ";
			co_return 0;
		}

		if (responce.result != MsgLoginResponce::OK)
		{
			LOG(WARNING) << "��¼ʧ��," << responce.result;
			SendToGateClient(responce, (uint64_t)this);
			if (co_await CoTimer::Wait(1s, funCancel))
				co_return 0;

			m_refSession.m_refSession.CloseSocket();
			co_return 0;
		}
		m_bLoginOk = true;

	}

	{
		//��¼GameSvr
		SendToGameSvr<MsgGateAddSession>({ .nickName = StrConv::Utf8ToGbk(msg.name) }, GetId(), ++m_snSendToGameSvr);
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

void SendResponceToWorldSvr(const uint32_t rpcSnId, const uint64_t gateSessionId)
{
	MsgGateDeleteSessionResponce msgResponce;
	msgResponce.rpcSnId = rpcSnId;
	SendToWorldSvrת��(msgResponce, gateSessionId);
}

void GateSession::OnRecvWorldSvr(const MsgGateDeleteSession& msg)
{
	LOG(INFO) << "�����Ͽ���Ϸ�ͻ���Socket,Id=" << GetId(),
		m_refSession.m_refSession.CloseSocket();

	OnDestroy();
	SendResponceToWorldSvr(msg.rpcSnId, GetId());
}

bool GateSession::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		switch (msgId)
		{
		case MsgId::MsgId_Invalid_0://û����Ϣ�ɴ���
			return true;
		case MsgId::Login:return this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &GateSession::OnRecv); break;
		case MsgId::Gateת��:return this->m_MsgQueue.OnRecv���������(this->m_queueMsgGateת��, *this, &GateSession::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			_ASSERT(false);
			return false;
			break;
		}
		_ASSERT(false);
	}
	return true;
}
