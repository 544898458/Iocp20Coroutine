#pragma once
#include "WorldSession.h"
#include "../IocpNetwork/Sessions.h"

class WorldSvrAcceptGate
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSessionFromGame>;
	void OnAdd(CompeletionKeySession&);
	Sessions<WorldSessionFromGame> m_Sessions;
};

