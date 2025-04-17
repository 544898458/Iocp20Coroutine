#pragma once

#include "../CoRoutine/CoTask.h"
#include "../SpEntity.h"

class 苔蔓Component
{
public:
	苔蔓Component(Entity&);
	CoTaskBool Co苔蔓消亡();
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	int16_t m_i16半径 = 5;
	/// <summary>
	/// 如果附着建筑消失，就逐渐消亡
	/// </summary>
	WpEntity m_wp附着建筑;

private:
	FunCancel m_funCancel;
	/// <summary>
	/// 自己是苔蔓
	/// </summary>
	Entity& m_refEntity;
};