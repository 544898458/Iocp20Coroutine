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
	static void Sayϵͳ(Entity& refEntity, const std::string& str);
	static void ����Ի�(Space& refSpace, const std::string& refStrNickName,
		const std::string& strͷ����, const std::string& str������,
		const std::string& strͷ����, const std::string& str������,
		const std::string& str����, const bool b��ʾ�˳�������ť = false);
	static void ����Ի��ѿ���(const std::string& refStrNickName);
	static void ��������Buzz(Entity& refEntity, const std::string& str�ı�);
	static void ��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı� = "");
	static void ��������(UpPlayerComponent& refSp, const std::string& refStr����, const std::string& str�ı� = "");
	static void ��������(const std::string& refStrNickName, const std::string& refStr����, const std::string& str�ı�);
	static void Send��Դ(Entity& refEntity);
	template<class T> static void Send(const UpPlayerComponent& spPlayer���ܿ�, const T& ref);

	void Say(const std::string& str, const SayChannel channel);
	PlayerComponent(PlayerGateSession_Game& ref) :m_refSession(ref) {}
	PlayerGateSession_Game& m_refSession;
};
