#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"

class Entity;
//class PlayerGateSession_Game;
//class Space;

class ������Component
{
public:
	������Component(Entity &ref);
	static void AddComponet(Entity& refEntity);
	void TryCancel();
	void OnLoad();
	//void OnDestroy();
	//void OnBeforeDelayDelete();
	//void ��(Space& refSpace, Entity& refEntity);
	//void Update();
	//void ȫ�����ر�();
	CoTaskBool Co���׳�();
	Entity& m_refEntity;
	std::list<WpEntity> m_listWp�׳�;
	FunCancel m_Cancel���׳�;
};