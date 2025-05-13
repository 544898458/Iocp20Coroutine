#pragma once

#include "../CoRoutine/CoTask.h"
#include "../SpEntity.h"

struct Position;

class 苔蔓Component
{
public:
	static const uint16_t MAX半径 = 20;
	苔蔓Component(Entity&);
	static void AddComponent(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
	bool 在半径内(const Position& pos)const;
	bool 扩张结束() const;
	
	int16_t m_i16半径 = 5;
	/// <summary>
	/// 如果附着建筑消失，就逐渐消亡
	/// </summary>
	WpEntity m_wp附着建筑;

private:
	CoTaskBool Co萎缩消亡();
	CoTaskBool Co给周围加Buff();

	
	FunCancel安全 m_funCancel萎缩消亡;
	FunCancel安全 m_funCancel给周围加Buff;
	/// <summary>
	/// 自己是苔蔓
	/// </summary>
	Entity& m_refEntity;
};