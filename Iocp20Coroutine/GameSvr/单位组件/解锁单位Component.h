#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class ������λComponent
{
public:
	������λComponent(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	void ������λ(const ��λ���� ����);
	
private:
	CoTaskBool Co������λ(const ��λ���� ����);
	Entity& m_refEntity;
	FunCancel m_cancel������λ;
};