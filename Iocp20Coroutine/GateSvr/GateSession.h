#pragma once
#include "../IocpNetwork/WebSocketSession.h"
#include <deque>
class GateServer;
class GateSession
{
public:
	using CompeletionKeySession = WebSocketSession<GateSession>;
	GateSession(CompeletionKeySession& ref) :m_refSession(ref)
	{

	}
	//int OnRecv(CompeletionKeySession&, const void* buf, int len);
	/// <summary>
	/// WebSocket�յ�һ�����������ư�
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	void OnRecvWsPack(const void* buf, const int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, GateServer&);
	template<class T> std::deque<T>& GetQueue();
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
	CompeletionKeySession& m_refSession;
	//uint32_t m_snSend = 0;
};

