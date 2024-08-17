#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

enum MsgId;
class WorldClient;

class WorldClientMsgQueue
{
public:
	WorldClientMsgQueue()
	{
	}

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	template<class T>
	void Push(const T& msg);

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
	WorldClient* m_pWorldClient = nullptr;
private:
	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	static void OnRecv(WorldClientMsgQueue& refThis, const MsgSay& msg);
	static void OnRecv(WorldClientMsgQueue& refThis, const MsgConsumeMoneyResponce& msg);
	
	template<class T>
	std::deque<T>& GetQueue();
	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����,�����л��������̣߳�����������Ϣ���߼��߳�
	/// </summary>
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgConsumeMoneyResponce> m_queueConsumeMoneyResponce;
	MsgQueue m_MsgQueue;
	
};
class WorldClientSession
{
public:
	void OnInit(Iocp::SessionSocketCompletionKey<WorldClientSession>& session, WorldClient& refWorldClient)
	{
		m_pWorldClient = &refWorldClient;
		m_pSession = &session;
		session.Session.m_MsgQueue.m_pWorldClient = &refWorldClient;
	}

	/// <summary>
	///
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns>�����Ѵ�����ֽ�������Щ���ݽ����̴ӽ��ܻ�����ɾ��</returns>
	int OnRecv(Iocp::SessionSocketCompletionKey<WorldClientSession>& refSession, const void* buf, int len)
	{
		const void* bufPack(nullptr);
		int lenPack(0);
		std::tie(bufPack, lenPack) = Iocp::OnRecv2(buf, len);
		if (lenPack > 0 && nullptr != bufPack)
		{
			OnRecvPack(bufPack, lenPack);
		}

		return lenPack;
	}
	void OnRecvPack(const void* buf, int len);
	void OnDestroy()
	{

	}
	template<class T>
	void PushMsg(const msgpack::object& obj);

	WorldClient* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<WorldClientSession> *m_pSession = nullptr;
	/// <summary>
	/// ���������Ϣ���У�������Ϣ����ɶ˿��̣߳�������Ϣ�����̣߳�����̨�����̣߳�
	/// </summary>
	WorldClientMsgQueue m_MsgQueue;
};
class WorldClient
{
public:
	void OnAdd(Iocp::SessionSocketCompletionKey<WorldClientSession>& session)
	{

	}
	static std::function<void(MsgSay const&)> m_funBroadcast;
};
