#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;

class 采集Component
{
public:
	void 采集(PlayerGateSession_Game&, Entity& refThis, WpEntity wp);
	CoTaskBool Co采集(PlayerGateSession_Game&, Entity& refThis, WpEntity wp);
	static void AddComponent(Entity& spEntity);
private:
	static constexpr uint32_t Max携带晶体矿() { return 5; }
	uint32_t m_u32携带晶体矿 = 0;
	FunCancel m_funCancel;
};

