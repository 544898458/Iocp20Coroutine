#pragma once
#include"../IocpNetwork/SessionSocketCompeletionKey.h"
#include "../LogStrategy/StrategyLog.h"
#include "../IocpNetwork/WebSocketSession.h"
#include <msgpack.hpp>
#include "../IocpNetwork/MsgPack.h"
class WorldServer;
class WorldSession
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompeletionKey < WorldSession >;
	int OnRecv(CompeletionKeySession&, const void* buf, int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, WorldServer&);
	template<class T>
	void Send(const T& ref) 
	{
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_pSession->Send(buf, len); });
	}
	CompeletionKeySession* m_pSession = nullptr;
	WorldServer* m_pServer = nullptr;
private:
	void OnRecvPack(const void* buf, int len);
};

