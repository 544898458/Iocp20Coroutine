#pragma once
#include "../CoRoutine/CoTask.h"
#include "../SpEntity.h"

class 无苔蔓就持续掉血Component
{
public:
	无苔蔓就持续掉血Component(Entity&);
	CoTaskBool Co苔蔓扩张();
	void TryCancel();
	static void AddComponent(Entity& refEntity);
	WpEntity m_wp苔蔓;

private:
	FunCancel m_funCancel;
	/// <summary>
	/// 自己是建筑，虫巢或者太岁
	/// </summary>
	Entity& m_refEntity;
};

