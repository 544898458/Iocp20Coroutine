#pragma once
#include "../MyMsgQueue.h"
class Entity;
class 飞向目标Component
{
public:
	static void AddComponent(Entity& refEntity, const Position& pos起始点, const Position& pos方向, const float f最远距离, const uint64_t idAttacker);
	飞向目标Component(Entity& ref, const Position& pos起始点, const Position& pos方向, const float f最远距离, const uint64_t idAttacker);
	void OnEntityDestroy(const bool bDestroy);
private:
	CoTaskBool Co飞向目标遇敌爆炸();
	Entity& m_refEntity;
	const Position m_vec方向;
	const Position m_pos起始点;
	const float m_f最远距离;
	FunCancel安全 m_funCancel;
	const uint64_t m_idAttacker;
};

