#include "pch.h"
#include "WorldClient.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"

//template Iocp::Server<WorldClient>;
//template bool Iocp::Server<WorldClient>::Init<WorldClientSession>(const uint16_t);
//template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<WorldClientSession>;
//std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;

/// <summary>
/// ���̣߳����߳�
/// </summary>
void WorldClientSession::Process()
{
	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//û����Ϣ�ɴ���
			break;

		switch (msgId)
		{
		case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &WorldClientSession::OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}
}


void WorldClientSession::OnRecv(const MsgLogin& msg)
{
	
}

template<> std::deque<MsgLogin>& WorldClientSession::GetQueue() { return m_queueLogin; }

/// <summary>
/// �����̣߳����̣߳�����
/// </summary>
/// <param name="refSession"></param>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <returns>�����Ѵ�����ֽ�������Щ���ݽ����̴ӽ��ܻ�����ɾ��</returns>

inline int WorldClientSession::OnRecv(Iocp::SessionSocketCompletionKey<WorldClientSession>& refSession, const void* buf, int len)
{
	return Iocp::OnRecv3(buf, len, *this, &WorldClientSession::OnRecvPack);
}

/// <summary>
/// �����̣߳����̣߳�����
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void WorldClientSession::OnRecvPack(const void* buf, int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msg = MsgHead::GetMsgId(obj);
	//LOG(INFO) << obj;

	switch (msg.id)
	{
	case MsgId::Login:	m_MsgQueue.PushMsg<MsgLogin>(*this,obj);break;
	default:
		LOG(WARNING) << "ERR:" << msg.id;
		break;
	}
}