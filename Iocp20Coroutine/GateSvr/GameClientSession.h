#pragma once
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "../IocpNetwork/MsgPack.h"

/// <summary>
/// 自己是客户端，连上GameSvr
/// </summary>
class GameClientSession
{
public:
	using CompletetionKeySession = Iocp::SessionSocketCompletionKey<GameClientSession>;
	GameClientSession(CompletetionKeySession& refSession):m_refSession(refSession){}
	int OnRecv(CompletetionKeySession& refSession, const void* buf, int len);
	template<class T>
	void Send(const T& ref)
	{
		++m_snSend;
		ref.msg.SetSn(m_snSend);
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_refSession.Send(buf, len); });
	}
private:
	void OnRecvPack(const void* buf, const int len);
	CompletetionKeySession &m_refSession;
	uint32_t m_snSend = 0;
};

