#pragma once
#include "SpEntity.h"
#include <set>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class ������λ����Component
{
public:
	������λ����Component(Entity& ref);
	static void AddComponent(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
	void ����(const ��λ���� ��λ, const �������� ����);
	bool ��������()const;
private:
	CoTaskBool Co����(const ��λ���� ��λ, const �������� ����);
	Entity& m_refEntity;
	FunCancel m_cancel������λ����;
	std::map<��λ����, std::set<��������> > m_map��������λ����;
};