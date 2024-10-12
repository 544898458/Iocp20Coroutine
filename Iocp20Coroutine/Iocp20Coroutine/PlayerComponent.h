#pragma once
class GameSvrSession;
class PlayerGateSession_Game;
class PlayerComponent
{
public:
	void Say(const std::string &str);
	PlayerComponent(PlayerGateSession_Game&ref):m_refSession(ref){}
	PlayerGateSession_Game &m_refSession ;
};
