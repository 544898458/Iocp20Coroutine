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
	void OnEntityDestroy(const bool bDestroy);
	void ������λ(const ��λ���� ����);
	bool ���ڽ���()const;
private:
	CoTaskBool Co������λ(const ��λ���� ����);
	Entity& m_refEntity;
	FunCancel��ȫ m_cancel������λ;
};