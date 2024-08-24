#pragma once
#include "../IocpNetwork/SessionSocketCompletionKey.h"
class GameClientSession
{
public:
	int OnRecv(Iocp::SessionSocketCompletionKey<GameClientSession>& refSession, const void* buf, int len);
};

