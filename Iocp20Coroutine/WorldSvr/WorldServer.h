#pragma once
#include "WorldSession.h"
#include "../IocpNetwork/Sessions.h"
//#include "../IocpNetwork/WebSocketSession.h"
class WorldServer
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSession>;
	void OnAdd(CompeletionKeySession&);
	Sessions<WorldSession> m_Sessions;
};

