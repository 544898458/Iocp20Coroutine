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
	auto& refDb = *co_await DbPlayer::CoGet�������ؿ�(gbkName);
	MsgLoginResponce msgResponce;
	msgResponce.msg.rpcSnId = msg.msg.rpcSnId;
	if (refDb.pwd != msg.pwd)
	{
		msgResponce.result = MsgLoginResponce::PwdErr;
		SendToGateת��(msgResponce);
		co_return 0;
	}

	//����������ˣ�����Ҫ��Ҫ��ͬ��
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
		//֪ͨGameSvr��Session�Ƕ���״̬�����ٽ�����Ϣ��
		LOG(INFO) << gbkName << "," << m_idPlayerGateSession << "��" << refGateSession.m_idPlayerGateSession;
		refGateSession.SendToGateת��<MsgGateDeleteSession>({ .gateClientSessionId = refGateSession.m_idPlayerGateSession });

	}
	g_mapPlayerNickNameGateSessionId.insert({ gbkName,m_idPlayerGateSession });

	//֪ͨGateSvr������¼����
	SendToGateת��(msgResponce);
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
		LOG(ERROR) << "û����msgId:" << idMsg;
		assert(false);
		break;
	}
}

void PlayerGateSession_World::Process()
{

}


template<class T>
void PlayerGateSession_World::SendToGateת��(const T& refMsg)
{
	MsgPack::SendMsgpack(refMsg, [this](const void* buf, int len)
		{
			m_refSession.Send(MsgGateת��(buf, len, m_idPlayerGateSession, ++m_snSend));
		}, false);
}
template<class T>
void PlayerGateSession_World::SendToGameת��(const T& refMsg)
{
	MsgPack::SendMsgpack(refMsg, [this](const void* buf, int len)
		{
			//m_refSession.Send(MsgGateת��(buf, len, m_idPlayerGateSession, ++m_snSend));
		}, false);
}