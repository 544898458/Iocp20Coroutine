#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../IocpNetwork/MsgPack.h"

enum MsgId;
//class Client_GateToWorld;

class ClientSession_GateToWorld
{
public:
	using Session = Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>;
	ClientSession_GateToWorld(Session& ref) :m_refSession(ref) {}
	//void OnInit(Session& session, WorldClient& refWorldClient)
	//{
	//	m_pWorldClient = &refWorldClient;
	//	m_pSession = &session;
	//}

	/// <summary>
	/// �����̣߳����̣߳�����
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns>�����Ѵ�����ֽ�������Щ���ݽ����̴ӽ��ܻ�����ɾ��</returns>
	int OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>& refSession, const void* buf, int len);
	void OnDestroy()
	{
	}

	//Client_GateToWorld* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>* m_pSession = nullptr;

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
	template<class T> std::deque<T>& GetQueue();
	uint32_t m_snRecv = 0;
	template<class T>
	void Send(const T& ref)
	{
		++m_snSend;
		ref.msg.sn = (m_snSend);
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_refSession.Send(buf, len); });
	}
private:
	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgGateDeleteSession& msg);
	void OnRecv(const MsgGateת��& msg);
	void OnRecv(const Msg��������& msg);

	void OnRecvPack(const void* buf, int len);


	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����,�����л��������̣߳�����������Ϣ���߼��߳�
	/// </summary>
	std::deque<MsgGateDeleteSession> m_queueGateDeleteSession;
	std::deque<MsgGateת��> m_queueGateת��;
	std::deque<Msg��������> m_queue��������;
	MsgQueueMsgPack<ClientSession_GateToWorld> m_MsgQueue;
	Session& m_refSession;
	uint32_t m_snSend = 0;
};
//class Client_GateToWorld
//{
//public:
//	void OnAdd(Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>& session)
//	{
//
//	}
//};
