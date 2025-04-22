#pragma once
#include "../MyMsgQueue.h"
class Entity;
class 飞向目标Component
{
public:
	static void AddComponent(Entity& refEntity, const Position& pos起始点, const Position& pos方向, const float f最远距离);
	飞向目标Component(Entity& ref, const Position& pos起始点, const Position& pos方向, const float f最远距离);
	void OnEntityDestroy(const bool bDestroy);
private:
	CoTaskBool Co飞向目标遇敌爆炸();
	Entity& m_refEntity;
	const Position m_vec方向;
	const Position m_pos起始点;
	const float m_f最远距离;
	FunCancel m_funCancel;
};

