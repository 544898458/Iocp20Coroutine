#include "pch.h"
#include "ClientSession_GateToGame.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/ThreadPool.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
//template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<ClientSession_GateToGame>;
//std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;
template<> std::deque<MsgGateSvrת��GameSvr��Ϣ����Ϸǰ��>& ClientSession_GateToGame::GetQueue() { return m_queueMsgGateSvrת��GameSvr��Ϣ����Ϸǰ��; }

int ClientSession_GateToGame::OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GateToGame>& refSession, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &ClientSession_GateToGame::OnRecvPack);
}

template<class T>
void SendToGateClient(const T& refMsg, uint64_t gateSessionId);

/// <summary>
/// �˴��������߳�
/// GateSvr�յ�����GameSvr����Ϣ
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void ClientSession_GateToGame::OnRecvPack(const void* buf, const int len)
{

	try
	{
		msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try

		msgpack::object obj = oh.get();
		const auto msg = MsgHead::GetMsgId(obj);
		//LOG(INFO) << obj;
		//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
		//auto pSessionSocketCompeletionKey = this->m_pWsSession;
		switch (msg.id)
		{
			//case MsgId::Login:m_MsgQueue.PushMsg<MsgLogin>(*this, obj); break;
			//case MsgId::Move:m_MsgQueue.PushMsg<MsgMove>(*this, obj); break;
			//case MsgId::Say:m_MsgQueue.PushMsg<MsgSay >(*this, obj); break;
			//case MsgId::SelectRoles:m_MsgQueue.PushMsg<MsgSelectRoles>(*this, obj); break;
			//case MsgId::AddRole:m_MsgQueue.PushMsg<MsgAddRole>(*this, obj); break;
			//case MsgId::AddBuilding:m_MsgQueue.PushMsg<MsgAddBuilding>(*this, obj); break;
		case MsgId::Gateת��:
		{
			const auto msg = obj.as<MsgGateת��>();
			if (msg.vecByte.empty())
			{
				LOG(ERROR) << "ERR";
				_ASSERT(false);
				return;
			}
			MsgGateSvrת��GameSvr��Ϣ����Ϸǰ�� msg��ǰ��(&msg.vecByte[0], (int)msg.vecByte.size());
			msg��ǰ��.idGateClientSession = msg.gateClientSessionId;
			//SendToGateClient(msg��ǰ��, msg.gateClientSessionId);
			m_MsgQueue.PushMsg<MsgGateSvrת��GameSvr��Ϣ����Ϸǰ��>(*this, msg��ǰ��); break;
		}
		break;
		default:
			LOG(ERROR) << "û����msgId:" << msg.id;
			_ASSERT(false);
			break;
		}
	}
	catch (const msgpack::type_error& error)
	{
		LOG(ERROR) << len << "=len, ClientSession_GateToGame,�����л�ʧ��," << error.what();
		_ASSERT(false);
		return;
	}
}



template<class T>
inline void ClientSession_GateToGame::Send(const T& ref)
{
	//�жϴ˴������߳�
	CHECK_RET_VOID(Iocp::ThreadPool::Is�����߳�());

	++m_snSend;
	ref.msg.sn = m_snSend;
	//LOG(INFO) << "ClientSession_GateToGame,m_snSend=" << m_snSend;
	MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_refSession.Send(buf, len); });
}
template void ClientSession_GateToGame::Send(const MsgGateת��& ref);


/// <summary>
/// ���̣߳����߳�
/// </summary>
void ClientSession_GateToGame::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::MsgId_Invalid_0 == msgId)//û����Ϣ�ɴ���
			break;

		switch (msgId)
		{
		case MsgId::GateSvrת��GameSvr��Ϣ����Ϸǰ��:this->m_MsgQueue.OnRecv���������(this->m_queueMsgGateSvrת��GameSvr��Ϣ����Ϸǰ��, *this, &ClientSession_GateToGame::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			_ASSERT(false);
			break;
		}
	}
}
void ClientSession_GateToGame::OnRecv(const MsgGateSvrת��GameSvr��Ϣ����Ϸǰ��& msg)
{
	SendToGateClient(msg, msg.idGateClientSession);
}