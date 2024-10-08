#pragma once
#include "MyMsgQueue.h"
#include "SpEntity.h"
//#include "../IocpNetwork/WebSocketSession.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "PlayerGateSession_Game.h"
class GameSvr;
class Space;

class GameSvrSession
{
public:
	//using WebSocketGameSession = WebSocketSession<GameSvrSession>;
	using Session = Iocp::SessionSocketCompletionKey<GameSvrSession>;
	GameSvrSession(Session& ref):m_refSession(ref){}

	/// <summary>
	/// WebSocket�յ�һ�����������ư�
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	int OnRecv(Session&, const void* buf, const int len);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refWsSession"></param>
	/// <param name="server"></param>
	void OnInit(Session& refWsSession, GameSvr& server);
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
		m_refSession.Send(buf, len);
	}
	GameSvr* m_pServer = nullptr;

	template<class T>
	std::deque<T>& GetQueue();

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	bool Process();
	uint32_t m_snRecv = 0;

	Session& m_refSession;
private:
	void OnRecvPack(const void* buf, const int len);
	void OnRecv(const MsgGateת��& msg);
	void OnRecv(const MsgGateAddSession& msg, const uint64_t idGateClientSession);
	void OnRecv(const MsgGateDeleteSession& msg, const uint64_t idGateClientSession);

	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����
	/// </summary>
	std::deque<MsgGateת��> m_queueGateת��;
	//std::deque<MsgGateAddSession> m_queueGateAddSession;
	//std::deque<MsgGateDeleteSession> m_queueGateDeleteSession;

	std::map<uint64_t, PlayerGateSession_Game> m_mapPlayerGateSession;
	MsgQueueMsgPack<GameSvrSession> m_MsgQueue;
};
