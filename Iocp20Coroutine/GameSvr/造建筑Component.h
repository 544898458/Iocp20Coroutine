#pragma once
#include <unordered_set>
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;
class Entity;
enum ������λ����;
enum ���λ����;
struct Position;

/// <summary>
/// �˵�λ�����콨�������̳������䡢̽����
/// </summary>
class �콨��Component
{
public:
	static void AddComponent(Entity& refEntity, PlayerGateSession_Game& refGateSession, const ���λ���� ����);
	�콨��Component(PlayerGateSession_Game& refSession, Entity& refEntity, const ���λ���� ����);
	CoTaskBool Co�콨��(const Position refPos, const ������λ���� ����);
	CoTaskBool Co�������(WpEntity wpEntity����, FunCancel& cancel);
private:
	std::unordered_set<������λ����> m_set��������;
	FunCancel m_cancel�콨��;
	Entity& m_refEntity;
};

