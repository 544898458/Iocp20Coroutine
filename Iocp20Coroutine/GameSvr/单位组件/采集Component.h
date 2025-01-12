#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;
enum 单位类型;
class 采集Component
{
public:
	采集Component(Entity&);
	void 采集(PlayerGateSession_Game&, WpEntity wp);
	CoTaskBool Co采集(WpEntity wp);
	static void AddComponent(Entity& spEntity);
	CoTaskCancel m_TaskCancel;
private:
	static constexpr uint32_t Max携带矿() { return 5; }
	uint32_t m_u32携带矿 = 0;
	单位类型 m_携带矿类型;
	Entity& m_refEntity;
};

