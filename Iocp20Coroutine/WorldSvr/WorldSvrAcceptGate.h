#pragma once
#include "WorldSessionFromGate.h"
#include "../IocpNetwork/Sessions.h"

class WorldSvrAcceptGate
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSessionFromGate>;
	void OnAdd(CompeletionKeySession&);
	Sessions<WorldSessionFromGate> m_Sessions;
};

