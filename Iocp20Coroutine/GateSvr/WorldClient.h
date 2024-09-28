#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../IocpNetwork/MsgPack.h"

enum MsgId;
class WorldClient;

class WorldClientSession
{
public:
	using Session = Iocp::SessionSocketCompletionKey<WorldClientSession>;
	WorldClientSession(Session& ref) :m_refSession(ref) {}
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
	int OnRecv(Iocp::SessionSocketCompletionKey<WorldClientSession>& refSession, const void* buf, int len);
	void OnDestroy()
	{
	}

	WorldClient* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<WorldClientSession>* m_pSession = nullptr;

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
	void OnRecv(const MsgLogin& msg);

	void OnRecvPack(const void* buf, int len);


	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����,�����л��������̣߳�����������Ϣ���߼��߳�
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	MsgQueueMsgPack<WorldClientSession> m_MsgQueue;
	Session& m_refSession;
	uint32_t m_snSend = 0;
};
class WorldClient
{
public:
	void OnAdd(Iocp::SessionSocketCompletionKey<WorldClientSession>& session)
	{

	}
};
