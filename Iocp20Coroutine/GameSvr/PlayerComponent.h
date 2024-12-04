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
	static void 播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本="");
	static void Send资源(Entity& refEntity);
	void Say(const std::string& str, const SayChannel channel);
	PlayerComponent(PlayerGateSession_Game& ref) :m_refSession(ref) {}
	PlayerGateSession_Game& m_refSession;
};
