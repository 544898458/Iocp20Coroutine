#pragma once
#include "WorldSession.h"
#include "../IocpNetwork/Sessions.h"
//#include "../IocpNetwork/WebSocketSession.h"
class WorldSvrAcceptGame
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSessionFromGame>;
	void OnAdd(CompeletionKeySession&);
	Sessions<WorldSessionFromGame> m_Sessions;
};

