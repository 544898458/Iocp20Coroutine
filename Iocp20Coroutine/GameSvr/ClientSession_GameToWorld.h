#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"

enum MsgId;
//class Client_GateToWorld;

class ClientSession_GameToWorld
{
public:
	using Session = Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld>;
	ClientSession_GameToWorld(Session&) {}
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
	int OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld>& refSession, const void* buf, int len);
	void OnRecvPack(const void* buf, int len);
	void OnDestroy()
	{

	}
	
	//Client_GateToWorld* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld> *m_pSession = nullptr;
	
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
	void OnAppExit();

	template<class T> std::deque<T>& GetQueue();
	uint32_t m_snRecv = 0;

private:
	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgChangeMoneyResponce& msg);

	
	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����,�����л��������̣߳�����������Ϣ���߼��߳�
	/// </summary>
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgChangeMoneyResponce> m_queueConsumeMoneyResponce;
	MsgQueueMsgPack<ClientSession_GameToWorld> m_MsgQueue;
};