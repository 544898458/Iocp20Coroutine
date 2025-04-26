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
#include "../读配置文件/Try读Ini本地机器专用.h"

template class Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession>;
template class MsgQueueMsgPack<GateSession::CompeletionKeySession>;
template class WebSocketSession<GateSession>;
template class WebSocketEndpoint<GateSession, GateSession::CompeletionKeySession>;
template void WebSocketSession<GateSession>::OnInit<GateServer>(GateServer& server);
//int GateSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
//{
//    return 0;
//}

//void SendToGameSvr转发(const void* buf, const int len, uint64_t gateSessionId);// , uint32_t sn);
template<class T> void SendToWorldSvr转发(const T& refMsg, const uint64_t gateSessionId);// , uint32_t snSend);
template<class T> void SendToGameSvr(const T& refMsg, const uint64_t gateSessionId, uint32_t snSend);
template<> void SendToGameSvr(const MsgGate转发& refMsg, const uint64_t gateSessionId, uint32_t snSend);

void GateSession::OnRecvWsPack(const void* buf, const int len)
{
	try
	{
		msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
		msgpack::object obj = oh.get();
		const auto msg = MsgHead::GetMsgId(obj);
		//LOG(INFO) << obj;

		switch (msg.id)
		{
		case MsgId::Login:	m_MsgQueue.PushMsg<MsgLogin>(*this, obj); break;
		case MsgId::Gate转发:
			LOG(ERROR) << "黑客发来转发Gate转发:" << msg.id;
			break;
		default:
			//LOG(INFO) << "转发GameSvr消息:" << msg.id;
			//SendToGameSvr转发(buf, len, GetId());// , ++m_snSend);
			m_MsgQueue.PushMsg<MsgGate转发>(*this, MsgGate转发(buf, len, GetId(), 0)); break;
			break;
		}
	}
	catch (const msgpack::unpack_error& error)
	{
		LOG(WARNING) << "unpack_error,MsgPack解包失败:" << error.what();
		//_ASSERT(false);
	}
	catch (const msgpack::type_error& error)
	{
		LOG(WARNING) << "type_error,MsgPack解包失败:" << error.what();
		//_ASSERT(false);
	}

}
template<> std::deque<MsgLogin>& GateSession::GetQueue() { return m_queueLogin; }
template<> std::deque<MsgGate转发>& GateSession::GetQueue() { return m_queueMsgGate转发; }
void GateSession::OnRecv(const MsgLogin& msg)
{
	if (msg.name.empty())
	{
		m_refSession.m_refSession.CloseSocket();
	}
	LOG(INFO) << "玩家发来登录," << StrConv::Utf8ToGbk(msg.name);
	if (!m_coLogin.Finished() || m_bLoginOk)
	{
		LOG(ERROR) << "m_coLogin.Finished=" << m_coLogin.Finished() << ",m_bLoginOk=" << m_bLoginOk;
		return;
	}
	m_coLogin = CoLogin(msg, m_funCancelLogin);
	m_coLogin.Run();
}

void GateSession::OnRecv(const MsgGate转发& msg)
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

		uint32_t 版本号可进最高 = 0;
		uint32_t 版本号可进最低 = 0;
		Try读Ini本地机器专用(版本号可进最高, "GateSvr", "VerMax");
		Try读Ini本地机器专用(版本号可进最低, "GateSvr", "VerMin");
		if (版本号可进最高 < msg.u32版本号 || 版本号可进最低 > msg.u32版本号)
		{
			LOG(WARNING) << "[" << 版本号可进最高 << "," << 版本号可进最低 << "],版本:" << msg.u32版本号;
			SendToGateClient<MsgLoginResponce>({ .result = MsgLoginResponce::客户端版本不匹配, .str提示 = StrConv::GbkToUtf8("版本不匹配") }, (uint64_t)this);
			if (co_await CoTimer::Wait(1s, funCancel))
				co_return 0;

			m_refSession.m_refSession.CloseSocket();
			co_return 0;
		}
		auto [stop, responce] = co_await CoRpc<MsgLoginResponce>::Send<MsgLogin>(msg, [this](const MsgLogin& msg) {SendToWorldSvr转发<MsgLogin>(msg, GetId()); }, funCancel);
		LOG(INFO) << "WorldSvr返回登录结果stop=" << stop << ",error=" << responce.result;
		if (stop)
		{
			LOG(WARNING) << "协程停止";
			co_return 0;
		}

		if (responce.result != MsgLoginResponce::OK)
		{
			LOG(WARNING) << "登录失败," << responce.result;
			SendToGateClient(responce, (uint64_t)this);
			if (co_await CoTimer::Wait(1s, funCancel))
				co_return 0;

			m_refSession.m_refSession.CloseSocket();
			co_return 0;
		}
		m_bLoginOk = true;

	}

	{
		//登录GameSvr
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
	SendToWorldSvr转发<MsgGateDeleteSession>({}, GetId());// , ++m_snSendToWorldSvr);
}

void GateSession::OnInit(GateServer& refServer)
{
	refServer.m_Sessions.AddSession(&m_refSession.m_refSession, [this, &refServer]()
		{
			LOG(INFO) << "游戏客户端已连上";
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
	SendToWorldSvr转发(msgResponce, gateSessionId);
}

void GateSession::OnRecvWorldSvr(const MsgGateDeleteSession& msg)
{
	LOG(INFO) << "主动断开游戏客户端Socket,Id=" << GetId(),
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
		case MsgId::MsgId_Invalid_0://没有消息可处理
			return true;
		case MsgId::Login:return this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &GateSession::OnRecv); break;
		case MsgId::Gate转发:return this->m_MsgQueue.OnRecv不处理序号(this->m_queueMsgGate转发, *this, &GateSession::OnRecv); break;
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
