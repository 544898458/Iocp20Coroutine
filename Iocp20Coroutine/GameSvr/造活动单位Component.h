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
	static void AddComponent(Entity& refEntity, const ������λ���� ����);
	����λComponent(Entity& refEntity, const ������λ���� ����);
	void ���(PlayerGateSession_Game&, Entity& refEntity, const ���λ���� ����);
	void TryCancel(Entity& refEntity);
	uint16_t �ȴ���Count()const;
private:
	CoTaskBool Co����λ();
	
private:
	CoTaskCancel m_TaskCancel����λ;
	std::unordered_set<���λ����> m_set��������;
	std::list<���λ����> m_list�ȴ���;//int m_i�ȴ������ = 0;
	Entity& m_refEntity;
};

