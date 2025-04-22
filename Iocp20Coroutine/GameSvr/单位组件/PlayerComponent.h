#pragma once
#include "../SpPlayerComponent.h"
class GameSvrSession;
class PlayerGateSession_Game;
class Entity;
class Space;
enum SayChannel;

class PlayerComponent;
class PlayerComponent
{
public:
	static void AddComponent(Entity& refEntity, UpPlayerComponent& wpSession, const std::string& strNickName);
	static void AddComponent(Entity& refEntity, PlayerGateSession_Game& refSession);
	static void Say(Entity& refEntity, const std::string& str, const SayChannel channel);
	static void Say系统(Entity& refEntity, const std::string& str);
	static void 剧情对话(Space& refSpace, const std::string& refStrNickName,
		const std::string& str头像左, const std::string& str名字左,
		const std::string& str头像右, const std::string& str名字右,
		const std::string& str内容, const bool b显示退出场景按钮 = false);
	static void 剧情对话已看完(const std::string& refStrNickName);
	static void 播放声音Buzz(Entity& refEntity, const std::string& str文本);
	static void 播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本 = "");
	static void 播放声音(UpPlayerComponent& refSp, const std::string& refStr声音, const std::string& str文本 = "");
	static void 播放声音(const std::string& refStrNickName, const std::string& refStr声音, const std::string& str文本);
	static void Send资源(Entity& refEntity);
	template<class T> static void Send(const UpPlayerComponent& spPlayer可能空, const T& ref);

	void Say(const std::string& str, const SayChannel channel);
	PlayerComponent(PlayerGateSession_Game& ref) :m_refSession(ref) {}
	PlayerGateSession_Game& m_refSession;
};
