#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
class Entity;


class 虫巢Component
{
public:
	虫巢Component(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	void OnLoad();
	void Set集结点(const Position& pos);
	//void OnDestroy();
	//void OnBeforeDelayDelete();
	//void 进(Space& refSpace, Entity& refEntity);
	//void Update();
	//void 全都出地堡();
	CoTaskBool Co造幼虫();
	Entity& m_refEntity;
	std::list<WpEntity> m_listWp幼虫;
	FunCancel m_Cancel造幼虫;
private:
	Position m_pos幼虫集结点;
};