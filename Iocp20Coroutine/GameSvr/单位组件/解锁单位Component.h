#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class 解锁单位Component
{
public:
	解锁单位Component(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	void 解锁单位(const 单位类型 类型);
	
private:
	CoTaskBool Co解锁单位(const 单位类型 类型);
	Entity& m_refEntity;
	FunCancel m_cancel解锁单位;
};