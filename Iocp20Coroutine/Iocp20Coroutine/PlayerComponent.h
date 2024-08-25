#pragma once
class GameSvrSession;
class PlayerGateSession;
class PlayerComponent
{
public:
	PlayerComponent(PlayerGateSession&ref):m_refSession(ref){}
	PlayerGateSession &m_refSession ;
};
