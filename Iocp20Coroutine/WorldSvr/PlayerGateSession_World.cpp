#include "pch.h"
#include "PlayerGateSession_World.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "WorldSessionFromGate.h"
#include "DbPlayer.h"
#include "../慢操作AliyunGreen/慢操作AliyunGreen.h"


void PlayerGateSession_World::OnDestroy()
{

}

std::map<std::string, uint64_t> g_mapPlayerNickNameGateSessionId;
extern std::map<uint64_t, PlayerGateSession_World> g_mapPlayerGateSession;

template<typename T> void BroadcastToGate(const T& refMsg);
void BroadcastToGate在线人数();
CoTask<int> PlayerGateSession_World::CoLogin(const MsgLogin msg)
{
	auto utf8Name = msg.name;
	auto gbkName = StrConv::Utf8ToGbk(msg.name);
	MsgLoginResponce msgResponce;
	msgResponce.msg.rpcSnId = msg.msg.rpcSnId;
	if (gbkName.empty())
	{
		msgResponce.result = MsgLoginResponce::NameErr;
		SendToGate转发(msgResponce);
		co_return 0;
	}
	extern 慢操作AliyunGreen g_慢操作AliyunGreen;
	static FunCancel funCancle;

	if (!co_await g_慢操作AliyunGreen.CoAliyunGreen(gbkName, funCancle))
	{
		msgResponce.result = MsgLoginResponce::NameErr;
		msgResponce.str提示 = "请换一个名字";
		SendToGate转发(msgResponce);
		co_return 0;
	}

	auto& refDb = *co_await DbPlayer::CoGet绝不返回空(gbkName);
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
			LOG(ERROR) << "idSessionId=" << idSessionId;
			_ASSERT(false);
			co_return 0;
		}

		auto& refGateSession = itFindSession->second;
		//通知GameSvr此Session是断线状态（不再接收消息）
		LOG(INFO) << gbkName << "," << m_idPlayerGateSession << "踢" << refGateSession.m_idPlayerGateSession;
		co_await CoRpc<MsgGateDeleteSessionResponce>::Send<MsgGateDeleteSession>({}, [&refGateSession](const MsgGateDeleteSession& refMsg) {refGateSession.SendToGate转发(refMsg); }, funCancle);
		_ASSERT(g_mapPlayerNickNameGateSessionId.find(gbkName) == g_mapPlayerNickNameGateSessionId.end());
	}
	const auto pair = g_mapPlayerNickNameGateSessionId.insert({ gbkName, m_idPlayerGateSession });//g_mapPlayerNickNameGateSessionId[gbkName] = m_idPlayerGateSession;
	_ASSERT(pair.second);
	LOG(INFO) << gbkName << ",已添加此名字对应的SessionId=" << m_idPlayerGateSession;

	m_nickName = gbkName;
	//通知GateSvr继续登录流程
	SendToGate转发(msgResponce);

	BroadcastToGate在线人数();//BroadcastToGate<Msg在线人数>({.u16人数 = (uint16_t)g_mapPlayerNickNameGateSessionId.size()});
	co_return 0;
}

void PlayerGateSession_World::OnRecv(const MsgLogin& msg)
{
	_ASSERT(m_coLogin.Finished());
	m_coLogin = CoLogin(msg);
	m_coLogin.Run();
}

void PlayerGateSession_World::OnRecv(const MsgGateDeleteSessionResponce& msg)
{
	CoRpc<MsgGateDeleteSessionResponce>::OnRecvResponce(false, msg);
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
	case MsgId::GateDeleteSessionResponce:RecvMsg<MsgGateDeleteSessionResponce>(obj); break;
	default:
		LOG(ERROR) << "没处理msgId:" << idMsg;
		_ASSERT(false);
		break;
	}
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