#pragma once
#include "../CoRoutine/CoTask.h"
#include <unordered_set>
#include "SpEntity.h"
#include "../MyMsgQueue.h"

enum ��λ����;
enum ��λ����;
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
	static void AddComponent(Entity& refEntity);
	����λComponent(Entity& refEntity);
	bool ����(const ��λ���� ����);
	void ���(PlayerGateSession_Game&, const ��λ���� ����);
	bool Is�׳������ɱ�();
	void TryCancel(Entity& refEntity);
	uint16_t �ȴ���Count()const;
	Position m_pos�����;
	bool Is�׳�()const;
	void ������λ(const ��λ���� ����);
private:
	CoTaskBool Co������λ(const ��λ���� ����);
	CoTaskBool Co����λ();
	bool �ɼ�����㸽������Դ(Entity& refEntiy)const;
	
private:
	CoTaskCancel m_TaskCancel����λ;
	FunCancel m_cancel������λ;
	std::unordered_set<��λ����> m_set��������;
	std::list<��λ����> m_list�ȴ���;//int m_i�ȴ������ = 0;
	Entity& m_refEntity;
};

