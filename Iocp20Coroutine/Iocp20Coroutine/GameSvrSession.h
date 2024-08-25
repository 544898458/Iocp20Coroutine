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
	template<class T> void Send(const T& ref)
	{
		for (auto& pair : m_mapPlayerGateSession)
		{
			pair.second.Send(ref);
		}
	}
	GameSvr* m_pServer = nullptr;

	template<class T>
	std::deque<T>& GetQueue();

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
private:
	void OnRecvPack(const void* buf, const int len);
	void OnRecv(const MsgGateת��& msg);

	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����
	/// </summary>
	std::deque<MsgGateת��> m_queueGateת��;

	std::map<uint64_t, PlayerGateSession> m_mapPlayerGateSession;
	MsgQueueMsgPack<GameSvrSession> m_MsgQueue;
private:
	WebSocketGameSession* m_pWsSession = nullptr;
};
