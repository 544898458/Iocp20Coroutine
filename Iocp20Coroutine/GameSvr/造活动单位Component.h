#pragma once
#include "../CoRoutine/CoTask.h"
#include <unordered_set>
#include "SpEntity.h"

enum ���λ����;
enum ������λ����;
class PlayerGateSession_Game;
class Entity;
struct Position;

namespace ��λ
{
	struct ���λ����;
}

class ����λComponent
{
public:
	static void AddComponent(Entity& refEntity, PlayerGateSession_Game& refGateSession, const ������λ���� ����);
	����λComponent(PlayerGateSession_Game& refSession, Entity& refEntity, const ������λ���� ����);
	void ���(PlayerGateSession_Game&, Entity& refEntity, const ���λ���� ����);
	void TryCancel(Entity& refEntity);
	uint16_t �ȴ���Count()const;
	static SpEntity ����λ(PlayerGateSession_Game& refGateSession, const Position& pos, const ��λ::���λ����& ����, const ���λ���� ����);
private:
	CoTaskBool Co����λ(PlayerGateSession_Game& refGateSession);
	
private:
	CoTaskCancel m_TaskCancel����λ;
	std::unordered_set<���λ����> m_set��������;
	std::list<���λ����> m_list�ȴ���;//int m_i�ȴ������ = 0;
	Entity& m_refEntity;
};

