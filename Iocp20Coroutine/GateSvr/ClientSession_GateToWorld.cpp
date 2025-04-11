#include "pch.h"
#include "ClientSession_GateToWorld.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "GateServer.h"

template Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>;

/// <summary>
/// ���̣߳����߳�
/// </summary>
void ClientSession_GateToWorld::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::MsgId_Invalid_0 == msgId)//û����Ϣ�ɴ���
			break;

		switch (msgId)
		{
		case MsgId::Gateת��:this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &ClientSession_GateToWorld::OnRecv); break;
		case MsgId::��������:this->m_MsgQueue.OnRecv(this->m_queue��������, *this, &ClientSession_GateToWorld::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			_ASSERT(false);
			break;
		}
	}
}

void ClientSession_GateToWorld::OnRecv(const MsgGateת��& msgת��)
{
	if (msgת��.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		_ASSERT(false);
		return;
	}

	msgpack::object_handle oh = msgpack::unpack((const char*)&msgת��.vecByte[0], msgת��.vecByte.size());//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	LOG(INFO) << obj;
	extern std::unique_ptr<Iocp::Server<GateServer>> g_upGateSvr;
	if (!g_upGateSvr->m_Server.m_Sessions.GetSession(msgת��.gateClientSessionId, [&msg, &obj](auto& refGateSession) {
		switch (msg.id)
		{
		case MsgId::Login:	refGateSession.Session.m_Session.OnRecvWorldSvr(obj.as<MsgLoginResponce>());	break;
		case MsgId::GateDeleteSession:	refGateSession.Session.m_Session.OnRecvWorldSvr(obj.as<MsgGateDeleteSession>());	break;
		}
		}))
	{
		switch (msg.id)
		{
		case MsgId::GateDeleteSession:
			LOG(WARNING) << "WorldSvr ���� GateSvr ɾ����� GateSession ʱ,�� Session ���Ѷ���,ֱ�ӷ���ɾ���ɹ�,gateClientSessionId=" << msgת��.gateClientSessionId;
			void SendResponceToWorldSvr(const uint32_t rpcSnId, const uint64_t gateSessionId);
			SendResponceToWorldSvr(msg.rpcSnId, msgת��.gateClientSessionId);
			break;
		default:
			LOG(ERROR) << msg.id << "=msg.id,msgת��.gateClientSessionId=" << msgת��.gateClientSessionId;
			break;
		}
	}
}
void BroadToGateClient(const MsgGateSvrת��WorldSvr��Ϣ����Ϸǰ��& refMsg);
void ClientSession_GateToWorld::OnRecv(const Msg��������& msg��������)
{
	MsgPack::SendMsgpack(msg��������, [](const void* buf, int len)
		{
			MsgGateSvrת��WorldSvr��Ϣ����Ϸǰ�� msg��ǰ��(buf, len);
			BroadToGateClient(msg��ǰ��);
		}, false);

}

template<> std::deque<MsgGateDeleteSession>& ClientSession_GateToWorld::GetQueue() { return m_queueGateDeleteSession; }
template<> std::deque<MsgGateת��>& ClientSession_GateToWorld::GetQueue() { return m_queueGateת��; }
template<> std::deque<Msg��������>& ClientSession_GateToWorld::GetQueue() { return m_queue��������; }

/// <summary>
/// �����̣߳����̣߳�����
/// </summary>
/// <param name="refSession"></param>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <returns>�����Ѵ�����ֽ�������Щ���ݽ����̴ӽ��ܻ�����ɾ��</returns>

inline int ClientSession_GateToWorld::OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>& refSession, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &ClientSession_GateToWorld::OnRecvPack);
}

/// <summary>
/// �����̣߳����̣߳�����
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void ClientSession_GateToWorld::OnRecvPack(const void* buf, int len)
{
	try
	{
		msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
		msgpack::object obj = oh.get();
		const auto msg = MsgHead::GetMsgId(obj);
		//LOG(INFO) << obj;

		switch (msg.id)
		{
		case MsgId::GateDeleteSession:	m_MsgQueue.PushMsg<MsgGateDeleteSession>(*this, obj); break;
		case MsgId::Gateת��:	m_MsgQueue.PushMsg<MsgGateת��>(*this, obj); break;
		case MsgId::��������:	m_MsgQueue.PushMsg<Msg��������>(*this, obj); break;
		default:
			LOG(WARNING) << "ERR:" << msg.id;
			break;
		}
	}
	catch (const msgpack::type_error& error)
	{
		LOG(ERROR) << len << "=len, ClientSession_GateToWorld,�����л�ʧ��," << error.what();
		_ASSERT(false);
		return;
	}
}