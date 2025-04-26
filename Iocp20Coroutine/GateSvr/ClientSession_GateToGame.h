#pragma once
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../GameSvr/MyMsgQueue.h"
#include <deque>
/// <summary>
/// 自己是客户端，连上GameSvr
/// </summary>
class ClientSession_GateToGame
{
public:
	using CompletetionKeySession = Iocp::SessionSocketCompletionKey<ClientSession_GateToGame>;
	ClientSession_GateToGame(CompletetionKeySession& refSession):m_refSession(refSession){}
	int OnRecv(CompletetionKeySession& refSession, const void* buf, int len);
	template<class T> std::deque<T>& GetQueue();

	template<class T>
	void Send(const T& ref);
	void Process();
private:
	void OnRecvPack(const void* buf, const int len);
	void OnRecv(const MsgGateSvr转发GameSvr消息给游戏前端& msg);
	CompletetionKeySession &m_refSession;
	MsgQueueMsgPack<ClientSession_GateToGame> m_MsgQueue;
	std::deque<MsgGateSvr转发GameSvr消息给游戏前端> m_queueMsgGateSvr转发GameSvr消息给游戏前端;

	uint8_t m_snSend = 0;
};