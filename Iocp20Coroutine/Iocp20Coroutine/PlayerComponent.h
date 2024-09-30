#pragma once
class GameSvrSession;
class PlayerGateSession_Game;
class PlayerComponent
{
public:
	PlayerComponent(PlayerGateSession_Game&ref):m_refSession(ref){}
	PlayerGateSession_Game &m_refSession ;
};
