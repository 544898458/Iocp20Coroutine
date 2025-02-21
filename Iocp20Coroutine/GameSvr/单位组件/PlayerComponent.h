#pragma once
class GameSvrSession;
class PlayerGateSession_Game;
class Entity;
enum SayChannel;

class PlayerComponent;
typedef std::shared_ptr<PlayerComponent> SpPlayerComponent;
class PlayerComponent
{
public:
	static void AddComponent(Entity& refEntity, std::weak_ptr<PlayerComponent> wpSession, const std::string& strNickName);
	static void AddComponent(Entity& refEntity, PlayerGateSession_Game& refSession);
	static void Say(Entity& refEntity, const std::string& str, const SayChannel channel);
	static void Say系统(Entity& refEntity, const std::string& str);
	static void 剧情对话(const std::string& refStrNickName,
		const std::string& str头像左, const std::string& str名字左,
		const std::string& str头像右, const std::string& str名字右,
		const std::string& str内容, const bool b显示退出场景按钮 = false);
	static void 剧情对话已看完(const std::string& refStrNickName);
	static void 播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本 = "");
	static void 播放声音(SpPlayerComponent& refSp, const std::string& refStr声音, const std::string& str文本 = "");
	static void 播放声音(const std::string& refStrNickName, const std::string& refStr声音, const std::string& str文本);
	static void Send资源(Entity& refEntity);
	template<class T> static void Send(const SpPlayerComponent& spPlayer可能空, const T& ref);

	void Say(const std::string& str, const SayChannel channel);
	PlayerComponent(PlayerGateSession_Game& ref) :m_refSession(ref) {}
	PlayerGateSession_Game& m_refSession;
};
