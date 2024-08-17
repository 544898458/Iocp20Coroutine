#include "StdAfx.h"
#include "WorldClient.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/MsgQueueTemplate.h"

template Iocp::Server<WorldClient>;
//template bool Iocp::Server<WorldClient>::Init<WorldClientSession>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<WorldClientSession>;
std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;

//���̣߳����߳�
void WorldClientMsgQueue::Process()
{
	const MsgId msgId = this->m_MsgQueue.PopMsg();
	switch (msgId)
	{
	case MsgId::Invalid_0://û����Ϣ�ɴ���
		return;
	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, OnRecv); break;
	case MsgId::ConsumeMoneyResponce:this->m_MsgQueue.OnRecv(this->m_queueConsumeMoneyResponce, *this, OnRecv); break;
	default:
		LOG(ERROR) << "msgId:" << msgId;
		assert(false);
		break;
	}
}


void WorldClientMsgQueue::OnRecv(WorldClientMsgQueue& refThis, const MsgSay& msg)
{
	LOG(INFO) << "WorldSvr��������:" << StrConv::Utf8ToGbk(msg.content);

	refThis.m_pWorldClient->m_funBroadcast(msg);
}

/// <summary>
/// ���̣߳����߳�
/// </summary>
/// <param name="refThis"></param>
/// <param name="msg"></param>
void WorldClientMsgQueue::OnRecv(WorldClientMsgQueue& refThis, const MsgConsumeMoneyResponce& msg)
{
	LOG(INFO) << "WorldSvr������Ǯ���,rpcSnId=" << msg.rpcSnId;
	CoRpc::OnRecvResponce(msg);
}

template<class T>
void WorldClientSession::PushMsg(const msgpack::object& obj)
{
	const auto msg = obj.as<T>();
	m_MsgQueue.Push(msg);
}


template<> std::deque<MsgSay>& WorldClientMsgQueue::GetQueue() { return m_queueSay; }
template<> std::deque<MsgConsumeMoneyResponce>& WorldClientMsgQueue::GetQueue() { return m_queueConsumeMoneyResponce; }
template<class T> void WorldClientMsgQueue::Push(const T& msg) { m_MsgQueue.Push(msg, GetQueue<T>()); }
template void WorldClientMsgQueue::Push(const MsgSay& msg);
template void WorldClientMsgQueue::Push(const MsgConsumeMoneyResponce& msg);


/// <summary>
/// ���������߳�
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void WorldClientSession::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
	LOG(INFO) << obj;

	switch (msgId)
	{
	case MsgId::Say:
	{
		PushMsg<MsgSay>(obj);

	}
	break;
	case MsgId::ConsumeMoneyResponce:
	{
		PushMsg<MsgConsumeMoneyResponce>(obj);


	}
	break;
	default:
		LOG(WARNING) << "ERR:" << msgId;
		break;
	}
}