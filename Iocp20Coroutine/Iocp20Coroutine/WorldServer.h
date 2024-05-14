#pragma once
#include "WorldSession.h"
#include "../IocpNetwork/Sessions.h"
class WorldServer
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompeletionKey<WorldSession>;
	void OnAdd(CompeletionKeySession&);
	Sessions<WorldSession> m_Sessions;
};

