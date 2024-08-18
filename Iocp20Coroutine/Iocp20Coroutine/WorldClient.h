#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

enum MsgId;
class WorldClient;

class WorldClientSession
{
public:
	void OnInit(Iocp::SessionSocketCompletionKey<WorldClientSession>& session, WorldClient& refWorldClient)
	{
		m_pWorldClient = &refWorldClient;
		m_pSession = &session;
		//session.Session.m_MsgQueue.m_pWorldClient = &refWorldClient;
	}

	/// <summary>
	/// �����̣߳����̣߳�����
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
	/// <summary>
	/// �����̣߳����̣߳�����
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="obj"></param>
	template<class T> void PushMsg(const msgpack::object& obj);

	WorldClient* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<WorldClientSession> *m_pSession = nullptr;
	
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
private:
	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgConsumeMoneyResponce& msg);

	template<class T>
	std::deque<T>& GetQueue();
	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����,�����л��������̣߳�����������Ϣ���߼��߳�
	/// </summary>
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgConsumeMoneyResponce> m_queueConsumeMoneyResponce;
	MsgQueue m_MsgQueue;
};
class WorldClient
{
public:
	void OnAdd(Iocp::SessionSocketCompletionKey<WorldClientSession>& session)
	{

	}
	static std::function<void(MsgSay const&)> m_funBroadcast;
};
