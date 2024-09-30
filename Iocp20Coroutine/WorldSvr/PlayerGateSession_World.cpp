#include "pch.h"
#include "PlayerGateSession_World.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "WorldSession.h"
//#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"

/// <summary>
/// GameSvrͨ��GateSvr͸������Ϸ�ͻ���
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="ref"></param>
template<class T>
void PlayerGateSession_World::Send(const T& ref)
{
	//++m_snSend;
	//ref.msg.sn = (m_snSend);

	//std::stringstream buffer;
	//msgpack::pack(buffer, ref);
	//buffer.seekg(0);

	//std::string str(buffer.str());
	//CHECK_GE_VOID(UINT16_MAX, str.size());
	//MsgGateת�� msg(str.data(), (int)str.size(), m_idPlayerGateSession, m_snSend);
	//MsgPack::SendMsgpack(msg, [this](const void* buf, int len)
	//	{
	//		this->m_refSession.SendToGate(buf, len);
	//	});
}


void PlayerGateSession_World::OnDestroy()
{

}

void PlayerGateSession_World::OnRecv(const MsgLogin& msg)
{
	auto utf8Name = msg.name;
	auto gbkName = StrConv::Utf8ToGbk(msg.name);
	MsgLoginResponce msgResponce = { .msg = {.rpcSnId = msg.msg.rpcSnId} };
	MsgPack::SendMsgpack(msg, [this](const void* buf, int len)
		{
			m_refSession.Send(MsgGateת��(buf, len, m_idPlayerGateSession, ++m_snSend));
		});
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
