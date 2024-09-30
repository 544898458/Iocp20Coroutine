#include "pch.h"
#include "WorldClient.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"

//template Iocp::Server<WorldClient>;
//template bool Iocp::Server<WorldClient>::Init<WorldClientSession>(const uint16_t);
//template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld>;
std::function<void(MsgSay const&)> m_funBroadcast;

/// <summary>
/// 主线程，单线程
/// </summary>
void ClientSession_GameToWorld::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//没有消息可处理
			break;

		switch (msgId)
		{
		case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &ClientSession_GameToWorld::OnRecv); break;
		case MsgId::ChangeMoneyResponce:this->m_MsgQueue.OnRecv(this->m_queueConsumeMoneyResponce, *this, &ClientSession_GameToWorld::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}
}


void ClientSession_GameToWorld::OnRecv(const MsgSay& msg)
{
	LOG(INFO) << "WorldSvr发来聊天:" << StrConv::Utf8ToGbk(msg.content);

	m_funBroadcast(msg);
}

/// <summary>
/// 主线程，单线程
/// </summary>
/// <param name="refThis"></param>
/// <param name="msg"></param>
void ClientSession_GameToWorld::OnRecv(const MsgChangeMoneyResponce& msg)
{
	//LOG(INFO) << "WorldSvr发来扣钱结果,rpcSnId=" << msg.rpcSnId;
	CoRpc<MsgChangeMoneyResponce>::OnRecvResponce(false,msg);
}


template<> std::deque<MsgSay>& ClientSession_GameToWorld::GetQueue() { return m_queueSay; }
template<> std::deque<MsgChangeMoneyResponce>& ClientSession_GameToWorld::GetQueue() { return m_queueConsumeMoneyResponce; }



/// <summary>
/// 网络线程（多线程）调用
/// </summary>
/// <param name="refSession"></param>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <returns>返回已处理的字节数，这些数据将立刻从接受缓冲中删除</returns>

inline int ClientSession_GameToWorld::OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld>& refSession, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &ClientSession_GameToWorld::OnRecvPack);
}

/// <summary>
/// 网络线程（多线程）调用
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void ClientSession_GameToWorld::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
	case MsgId::Say:m_MsgQueue.PushMsg<MsgSay>(*this,obj);break;
	case MsgId::ChangeMoneyResponce:m_MsgQueue.PushMsg<MsgChangeMoneyResponce>(*this, obj);break;
	default:
		LOG(WARNING) << "ERR:" << msg.id;
		break;
	}
}