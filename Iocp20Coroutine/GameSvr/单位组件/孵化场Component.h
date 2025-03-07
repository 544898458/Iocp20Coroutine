#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"

class Entity;
//class PlayerGateSession_Game;
//class Space;

class 孵化场Component
{
public:
	孵化场Component(Entity &ref);
	static void AddComponet(Entity& refEntity);
	void TryCancel();
	void OnLoad();
	//void OnDestroy();
	//void OnBeforeDelayDelete();
	//void 进(Space& refSpace, Entity& refEntity);
	//void Update();
	//void 全都出地堡();
	CoTaskBool Co造幼虫();
	Entity& m_refEntity;
	std::list<WpEntity> m_listWp幼虫;
	FunCancel m_Cancel造幼虫;
};