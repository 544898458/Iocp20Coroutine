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
	GameSvr();
	using Session = Iocp::SessionSocketCompletionKey<GameSvrSession>;
	void OnAdd(Session&);
	void OnDel();
	void Update();
	void OnAppExit();

	Sessions<GameSvrSession> m_Sessions;
};