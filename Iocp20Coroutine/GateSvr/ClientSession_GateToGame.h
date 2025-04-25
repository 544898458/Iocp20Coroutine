#pragma once
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "../IocpNetwork/MsgPack.h"

/// <summary>
/// 自己是客户端，连上GameSvr
/// </summary>
class ClientSession_GateToGame
{
public:
	using CompletetionKeySession = Iocp::SessionSocketCompletionKey<ClientSession_GateToGame>;
	ClientSession_GateToGame(CompletetionKeySession& refSession):m_refSession(refSession){}
	int OnRecv(CompletetionKeySession& refSession, const void* buf, int len);
	template<class T>
	void Send(const T& ref);
private:
	void OnRecvPack(const void* buf, const int len);
	CompletetionKeySession &m_refSession;
	uint32_t m_snSend = 0;
};