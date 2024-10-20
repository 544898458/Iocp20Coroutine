#pragma once
#include "../CoRoutine/CoTask.h"
#include <unordered_set>
enum 活动单位类型;
enum 建筑单位类型;
class PlayerGateSession_Game;
class Entity;

class 造活动单位Component
{
public:
	static void AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession, const 建筑单位类型 类型);
	造活动单位Component(PlayerGateSession_Game& refSession, Entity& refEntity, const 建筑单位类型 类型);
	void 造兵(PlayerGateSession_Game&, Entity& refEntity, const 活动单位类型 类型);
	void TryCancel(Entity& refEntity);
	uint16_t 等待造Count()const;
private:
	CoTaskBool Co造活动单位(PlayerGateSession_Game& refGateSession);
private:
	CoTaskCancel m_TaskCancel造活动单位;
	std::unordered_set<活动单位类型> m_set可造类型;
	std::list<活动单位类型> m_list等待造;//int m_i等待造兵数 = 0;
	Entity& m_refEntity;

};

