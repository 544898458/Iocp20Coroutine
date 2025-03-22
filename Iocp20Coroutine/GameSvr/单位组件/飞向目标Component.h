#pragma once
#include "../MyMsgQueue.h"
class Entity;
class 飞向目标Component
{
public:
	static void AddComponet(Entity& refEntity, const Position& pos目标);
	飞向目标Component(Entity& ref, const Position& pos目标);
	void TryCancel();
private:
	CoTaskBool Co飞向目标遇敌爆炸();
	Entity& m_refEntity;
	const Position m_pos目标;
	FunCancel m_funCancel;
};

