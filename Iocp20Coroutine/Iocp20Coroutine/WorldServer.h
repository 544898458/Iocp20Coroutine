#pragma once
#include "WorldSession.h"
#include "../IocpNetwork/Sessions.h"
#include "../IocpNetwork/WebSocketSession.h"
class WorldServer
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompeletionKey<WebSocketSession<WorldSession>>;
	void OnAdd(CompeletionKeySession&);
	Sessions< WebSocketSession< WorldSession> > m_Sessions;
};

