#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;

class �ɼ�Component
{
public:
	void �ɼ�(PlayerGateSession_Game&, Entity& refThis, WpEntity wp);
	CoTaskBool Co�ɼ�(PlayerGateSession_Game&, Entity& refThis, WpEntity wp);
	static void AddComponent(Entity& spEntity);
private:
	static constexpr uint32_t MaxЯ�������() { return 5; }
	uint32_t m_u32Я������� = 0;
	FunCancel m_funCancel;
};

