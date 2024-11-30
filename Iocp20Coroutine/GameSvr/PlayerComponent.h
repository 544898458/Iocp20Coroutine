#pragma once
class GameSvrSession;
class PlayerGateSession_Game;
class Entity;
enum SayChannel;

class PlayerComponent
{
public:
	static void AddComponent(Entity& refEntity, PlayerGateSession_Game& refSession);
	static void Say(Entity&refEntity, const std::string& str, const SayChannel channel);
	void Say(const std::string& str, const SayChannel channel);
	PlayerComponent(PlayerGateSession_Game& ref) :m_refSession(ref) {}
	PlayerGateSession_Game& m_refSession;
};
