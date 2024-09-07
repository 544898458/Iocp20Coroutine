#include "GameClientSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../Iocp20Coroutine/MyMsgQueue.h"

//template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<GameClientSession>;
//std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;

int GameClientSession::OnRecv(Iocp::SessionSocketCompletionKey<GameClientSession>& refSession, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &GameClientSession::OnRecvPack);
}

void SendToGateClient(const void* buf, const int len, uint64_t gateSessionId);

/// <summary>
/// 此处是网络线程
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void GameClientSession::OnRecvPack(const void* buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msgId = Msg::GetMsgId(obj);
	//LOG(INFO) << obj;
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msgId)
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
			assert(false);
			return;
		}
		SendToGateClient(&msg.vecByte[0], msg.vecByte.size(), msg.gateClientSessionId);
	}
	break;
	default:
		LOG(ERROR) << "没处理msgId:" << msgId;
		assert(false);
		break;
	}
}