#pragma once
#include "SpEntity.h"
#include <set>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class ������λ����Component
{
public:
	������λ����Component(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	void ����(const ��λ���� ��λ, const ��λ�������� ����);
	
private:
	CoTaskBool Co����(const ��λ���� ��λ, const ��λ�������� ����);
	Entity& m_refEntity;
	FunCancel m_cancel������λ����;
	std::map<��λ����, std::set<��λ��������> > m_map��������λ����;
};