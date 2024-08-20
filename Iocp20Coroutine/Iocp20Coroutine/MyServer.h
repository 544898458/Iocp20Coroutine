#pragma once
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../IocpNetwork/Sessions.h"

class GameSvr
{
public:
	GameSvr(const GameSvr&) = delete;
	GameSvr(GameSvr&&) = delete;
	GameSvr():m_space(this) {}
	using Session = Iocp::SessionSocketCompletionKey<WebSocketSession< GameSvrSession> >;
	void OnAdd(Session&);
	void OnDel();
	void Update();

	Space m_space;
	Sessions< WebSocketSession< GameSvrSession> > m_Sessions;

};