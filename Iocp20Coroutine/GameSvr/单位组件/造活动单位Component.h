#pragma once
#include "../CoRoutine/CoTask.h"
#include <unordered_set>
#include "SpEntity.h"
#include "../MyMsgQueue.h"

enum 单位类型;
enum 单位类型;
class PlayerGateSession_Game;
class Entity;
struct Position;

namespace 单位
{
	struct 活动单位配置;
}

class 造活动单位Component
{
public:
	static void AddComponent(Entity& refEntity);
	造活动单位Component(Entity& refEntity);
	bool 可造(const 单位类型 类型);
	bool 判断并提示缺少建筑(PlayerGateSession_Game& refGateSession, const 单位类型 类型);
	void 造兵(PlayerGateSession_Game&, const 单位类型 类型);
	bool Is幼虫正在蜕变();
	void TryCancel(Entity& refEntity);
	uint16_t 等待造Count()const;
	Position m_pos集结点;
	bool Is幼虫()const;
private:
	CoTaskBool Co造活动单位();
	bool 采集集结点附近的资源(Entity& refEntiy)const;
	
private:
	CoTaskCancel m_TaskCancel造活动单位;
	std::unordered_set<单位类型> m_set可造类型;
	std::list<单位类型> m_list等待造;//int m_i等待造兵数 = 0;
	Entity& m_refEntity;
};

