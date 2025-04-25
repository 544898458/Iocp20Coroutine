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
template<> std::deque<MsgGateSvr转发GameSvr消息给游戏前端>& ClientSession_GateToGame::GetQueue() { return m_queueMsgGateSvr转发GameSvr消息给游戏前端; }

int ClientSession_GateToGame::OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GateToGame>& refSession, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &ClientSession_GateToGame::OnRecvPack);
}

template<class T>
void SendToGateClient(const T& refMsg, uint64_t gateSessionId);

/// <summary>
/// 此处是网络线程
/// GateSvr收到来自GameSvr的消息
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void ClientSession_GateToGame::OnRecvPack(const void* buf, const int len)
{

	try
	{
		msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try

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
		case MsgId::Gate转发:
		{
			const auto msg = obj.as<MsgGate转发>();
			if (msg.vecByte.empty())
			{
				LOG(ERROR) << "ERR";
				_ASSERT(false);
				return;
			}
			MsgGateSvr转发GameSvr消息给游戏前端 msg给前端(&msg.vecByte[0], (int)msg.vecByte.size());
			msg给前端.idGateClientSession = msg.gateClientSessionId;
			//SendToGateClient(msg给前端, msg.gateClientSessionId);
			m_MsgQueue.PushMsg<MsgGateSvr转发GameSvr消息给游戏前端>(*this, msg给前端); break;
		}
		break;
		default:
			LOG(ERROR) << "没处理msgId:" << msg.id;
			_ASSERT(false);
			break;
		}
	}
	catch (const msgpack::type_error& error)
	{
		LOG(ERROR) << len << "=len, ClientSession_GateToGame,反序列化失败," << error.what();
		_ASSERT(false);
		return;
	}
}



template<class T>
inline void ClientSession_GateToGame::Send(const T& ref)
{
	//判断此处是主线程
	CHECK_RET_VOID(Iocp::ThreadPool::Is在主线程());

	++m_snSend;
	ref.msg.sn = m_snSend;
	//LOG(INFO) << "ClientSession_GateToGame,m_snSend=" << m_snSend;
	MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_refSession.Send(buf, len); });
}
template void ClientSession_GateToGame::Send(const MsgGate转发& ref);


/// <summary>
/// 主线程，单线程
/// </summary>
void ClientSession_GateToGame::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::MsgId_Invalid_0 == msgId)//没有消息可处理
			break;

		switch (msgId)
		{
		case MsgId::GateSvr转发GameSvr消息给游戏前端:this->m_MsgQueue.OnRecv不处理序号(this->m_queueMsgGateSvr转发GameSvr消息给游戏前端, *this, &ClientSession_GateToGame::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			_ASSERT(false);
			break;
		}
	}
}
void ClientSession_GateToGame::OnRecv(const MsgGateSvr转发GameSvr消息给游戏前端& msg)
{
	SendToGateClient(msg, msg.idGateClientSession);
}