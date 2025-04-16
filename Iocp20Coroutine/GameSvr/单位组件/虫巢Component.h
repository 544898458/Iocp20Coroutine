#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class �泲Component
{
public:
	�泲Component(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	void OnLoad();
	void Set�����(const Position& pos);
	//void OnDestroy();
	//void OnBeforeDelayDelete();
	//void ��(Space& refSpace, Entity& refEntity);
	//void Update();
	//void ȫ�����ر�();
	CoTaskBool Co���׳�();
	Entity& m_refEntity;
	std::list<WpEntity> m_listWp�׳�;
	FunCancel m_Cancel���׳�;
private:
	Position m_pos�׳漯���;
};