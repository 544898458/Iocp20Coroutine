#pragma once
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include <deque>
class GateServer;
class GateSession
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<GateSession>;
	int OnRecv(CompeletionKeySession&, const void* buf, int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, GateServer&);
	template<class T>
	std::deque<T>& GetQueue();
};

