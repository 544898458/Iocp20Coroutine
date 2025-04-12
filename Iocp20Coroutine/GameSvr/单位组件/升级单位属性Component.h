#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class 升级单位属性Component
{
public:
	升级单位属性Component(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	void 升级(const 单位类型 单位, const 单位属性类型 属性);
	
private:
	CoTaskBool Co升级(const 单位类型 单位, const 单位属性类型 属性);
	Entity& m_refEntity;
	FunCancel m_cancel升级单位属性;
};