#include "StdAfx.h"
#include "WorldClient.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"

//template Iocp::Server<WorldClient>;
//template bool Iocp::Server<WorldClient>::Init<WorldClientSession>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<WorldClientSession>;
std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;

/// <summary>
/// ���̣߳����߳�
/// </summary>
void WorldClientSession::Process()
{
	const MsgId msgId = this->m_MsgQueue.PopMsg();
	switch (msgId)
	{
	case MsgId::Invalid_0://û����Ϣ�ɴ���
		return;
	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &WorldClientSession::OnRecv); break;
	case MsgId::ChangeMoneyResponce:this->m_MsgQueue.OnRecv(this->m_queueConsumeMoneyResponce, *this, &WorldClientSession::OnRecv); break;
	default:
		LOG(ERROR) << "msgId:" << msgId;
		assert(false);
		break;
	}
}


void WorldClientSession::OnRecv(const MsgSay& msg)
{
	LOG(INFO) << "WorldSvr��������:" << StrConv::Utf8ToGbk(msg.content);

	m_pWorldClient->m_funBroadcast(msg);
}

/// <summary>
/// ���̣߳����߳�
/// </summary>
/// <param name="refThis"></param>
/// <param name="msg"></param>
void WorldClientSession::OnRecv(const MsgChangeMoneyResponce& msg)
{
	//LOG(INFO) << "WorldSvr������Ǯ���,rpcSnId=" << msg.rpcSnId;
	CoRpc<MsgChangeMoneyResponce>::OnRecvResponce(false,msg);
}


template<> std::deque<MsgSay>& WorldClientSession::GetQueue() { return m_queueSay; }
template<> std::deque<MsgChangeMoneyResponce>& WorldClientSession::GetQueue() { return m_queueConsumeMoneyResponce; }


/// <summary>
/// �����̣߳����̣߳�����
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void WorldClientSession::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
	//LOG(INFO) << obj;

	switch (msgId)
	{
	case MsgId::Say:m_MsgQueue.PushMsg<MsgSay>(*this,obj);break;
	case MsgId::ChangeMoneyResponce:m_MsgQueue.PushMsg<MsgChangeMoneyResponce>(*this, obj);break;
	default:
		LOG(WARNING) << "ERR:" << msgId;
		break;
	}
}