#pragma once
#include "WorldSession.h"
#include "../IocpNetwork/Sessions.h"

class WorldServerAcceptGate
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSessionFromGame>;
	void OnAdd(CompeletionKeySession&);
	Sessions<WorldSessionFromGame> m_Sessions;
};

