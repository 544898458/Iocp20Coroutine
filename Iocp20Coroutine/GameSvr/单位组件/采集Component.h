#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;
class 资源Component;

enum 单位类型;
class 采集Component
{
public:
	采集Component(Entity&);
	void 采集(WpEntity wp,bool 播放单位回应声音);
	WpEntity Get目标资源(WpEntity& refWp目标资源);
	CoTaskBool Co采集(WpEntity wp);
	static void AddComponent(Entity& spEntity);
	static bool 正在采集(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
	CoTaskCancel m_TaskCancel;
private:
	static constexpr uint32_t Max携带矿() { return 5; }
	bool 资源可采集(WpEntity& refWp目标资源);
	void 提醒资源枯竭();
	uint32_t m_u32携带矿 = 0;
	单位类型 m_携带矿类型;
	单位类型 m_目标资源类型;
	Entity& m_refEntity;
};

