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
	static void Sayϵͳ(Entity& refEntity, const std::string& str);
	static void ����Ի�(const std::string& refStrNickName,
		const std::string& strͷ����, const std::string& str������,
		const std::string& strͷ����, const std::string& str������,
		const std::string& str����, const bool b��ʾ�˳�������ť = false);
	static void ����Ի��ѿ���(const std::string& refStrNickName);
	static void ��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı� = "");
	static void ��������(SpPlayerComponent& refSp, const std::string& refStr����, const std::string& str�ı� = "");
	static void ��������(const std::string& refStrNickName, const std::string& refStr����, const std::string& str�ı�);
	static void Send��Դ(Entity& refEntity);
	template<class T> static void Send(const SpPlayerComponent& spPlayer���ܿ�, const T& ref);

	void Say(const std::string& str, const SayChannel channel);
	PlayerComponent(PlayerGateSession_Game& ref) :m_refSession(ref) {}
	PlayerGateSession_Game& m_refSession;
};
