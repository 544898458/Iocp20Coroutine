#pragma once
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "../IocpNetwork/Sessions.h"
#include "GateSession.h"
class GateServer
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession>;
	void OnAdd(CompeletionKeySession&);
	Sessions<GateSession::CompeletionKeySession > m_Sessions;
};


