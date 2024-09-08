#pragma once
#include "MyMsgQueue.h"
#include "SpEntity.h"
//#include "../IocpNetwork/WebSocketSession.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "PlayerGateSession.h"
class GameSvr;
class Space;

class GameSvrSession
{
public:
	//using WebSocketGameSession = WebSocketSession<GameSvrSession>;
	using WebSocketGameSession = Iocp::SessionSocketCompletionKey<GameSvrSession>;
	GameSvrSession(WebSocketGameSession& refWsSession) {}

	/// <summary>
	/// WebSocket�յ�һ�����������ư�
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	int OnRecv(WebSocketGameSession&, const void* buf, const int len);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refWsSession"></param>
	/// <param name="server"></param>
	void OnInit(WebSocketGameSession& refWsSession, GameSvr& server);
	void OnDestroy();
	template<class T> void Send(T&& ref)
	{
		for (auto& pair : m_mapPlayerGateSession)
		{
			pair.second.Send(ref);
		}
	}
	void SendToGate(const void* buf, const int len)
	{
		m_pWsSession->Send(buf, len);
	}
	GameSvr* m_pServer = nullptr;

	template<class T>
	std::deque<T>& GetQueue();

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
	uint32_t m_snRecv = 0;
private:
	void OnRecvPack(const void* buf, const int len);
	void OnRecv(const MsgGateת��& msg);
	void OnRecv(const MsgGateAddSession& msg);
	void OnRecv(const MsgGateDeleteSession& msg);

	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����
	/// </summary>
	std::deque<MsgGateת��> m_queueGateת��;
	std::deque<MsgGateAddSession> m_queueGateAddSession;
	std::deque<MsgGateDeleteSession> m_queueGateDeleteSession;

	std::map<uint64_t, PlayerGateSession> m_mapPlayerGateSession;
	MsgQueueMsgPack<GameSvrSession> m_MsgQueue;
private:
	WebSocketGameSession* m_pWsSession = nullptr;
};
