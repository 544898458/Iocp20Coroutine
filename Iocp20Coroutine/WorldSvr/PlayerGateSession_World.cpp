#include "pch.h"
#include "PlayerGateSession_World.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "WorldSessionFromGate.h"
#include "DbPlayer.h"



void PlayerGateSession_World::OnDestroy()
{

}

std::map<std::string, uint64_t> g_mapPlayerNickNameGateSessionId;
extern std::map<uint64_t, PlayerGateSession_World> g_mapPlayerGateSession;
CoTask<int> PlayerGateSession_World::CoLogin(const MsgLogin msg)
{
	auto utf8Name = msg.name;
	auto gbkName = StrConv::Utf8ToGbk(msg.name);
	auto& refDb = *co_await DbPlayer::CoGet绝不返回空(gbkName);
	MsgLoginResponce msgResponce;
	msgResponce.msg.rpcSnId = msg.msg.rpcSnId;
	if (refDb.pwd != msg.pwd)
	{
		msgResponce.result = MsgLoginResponce::PwdErr;
		SendToGate转发(msgResponce);
		co_return 0;
	}

	//现在密码对了，看看要不要踢同号
	auto itFindSessionId = g_mapPlayerNickNameGateSessionId.find(gbkName);
	if (g_mapPlayerNickNameGateSessionId.end() != itFindSessionId)
	{
		auto idSessionId = itFindSessionId->second;
		auto itFindSession = g_mapPlayerGateSession.find(idSessionId);
		if (g_mapPlayerGateSession.end() == itFindSession)
		{
			LOG(ERROR) << "idSessionI=" << idSessionId;
			assert(false);
			co_return 0;
		}

		auto& refGateSession = itFindSession->second;
		//通知GameSvr此Session是断线状态（不再接收消息）
		LOG(INFO) << gbkName << "," << m_idPlayerGateSession << "踢" << refGateSession.m_idPlayerGateSession;
		refGateSession.SendToGate转发<MsgGateDeleteSession>({ .gateClientSessionId = refGateSession.m_idPlayerGateSession });

	}
	g_mapPlayerNickNameGateSessionId.insert({ gbkName,m_idPlayerGateSession });

	//通知GateSvr继续登录流程
	SendToGate转发(msgResponce);
	co_return 0;
}

void PlayerGateSession_World::OnRecv(const MsgLogin& msg)
{
	assert(m_coLogin.Finished());
	m_coLogin = CoLogin(msg);
	m_coLogin.Run();
}

template<class T_Msg>
void PlayerGateSession_World::RecvMsg(const msgpack::object& obj)
{
	const auto msg = obj.as<T_Msg>();
	OnRecv(msg);
}

void PlayerGateSession_World::RecvMsg(const MsgId idMsg, const msgpack::object& obj)
{
	switch (idMsg)
	{
	case MsgId::Login:RecvMsg<MsgLogin>(obj); break;
	default:
		LOG(ERROR) << "没处理msgId:" << idMsg;
		assert(false);
		break;
	}
}

void PlayerGateSession_World::Process()
{

}


template<class T>
void PlayerGateSession_World::SendToGate转发(const T& refMsg)
{
	MsgPack::SendMsgpack(refMsg, [this](const void* buf, int len)
		{
			m_refSession.Send(MsgGate转发(buf, len, m_idPlayerGateSession, ++m_snSend));
		}, false);
}
template<class T>
void PlayerGateSession_World::SendToGame转发(const T& refMsg)
{
	MsgPack::SendMsgpack(refMsg, [this](const void* buf, int len)
		{
			//m_refSession.Send(MsgGate转发(buf, len, m_idPlayerGateSession, ++m_snSend));
		}, false);
}