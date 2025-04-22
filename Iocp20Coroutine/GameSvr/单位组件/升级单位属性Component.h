#pragma once
#include "SpEntity.h"
#include <set>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class 升级单位属性Component
{
public:
	升级单位属性Component(Entity& ref);
	static void AddComponent(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
	void 升级(const 单位类型 单位, const 属性类型 属性);
	bool 正在升级()const;
private:
	CoTaskBool Co升级(const 单位类型 单位, const 属性类型 属性);
	Entity& m_refEntity;
	FunCancel m_cancel升级单位属性;
	std::map<单位类型, std::set<属性类型> > m_map可升级单位属性;
};