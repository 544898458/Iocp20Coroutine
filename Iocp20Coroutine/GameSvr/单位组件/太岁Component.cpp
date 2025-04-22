#include "pch.h"
#include "太岁Component.h"
#include "苔蔓Component.h"
#include "苔蔓扩张Component.h"
#include "造建筑Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"
#include "BuffComponent.h"
#include "../枚举/BuffId.h"
#include "../PlayerGateSession_Game.h"

void 太岁Component::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up太岁) << "m_up太岁";
	refEntity.m_up太岁.reset(new 太岁Component(refEntity));
}

void 太岁Component::分裂(const Position& refPos)
{
	if (!m_wp分裂.expired())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "不能再分裂");
		return;
	}

	CHECK_RET_VOID(m_refEntity.m_up苔蔓扩张);
	CHECK_WP_RET_VOID(m_refEntity.m_up苔蔓扩张->m_wp苔蔓);
	const auto& ref苔蔓 = *m_refEntity.m_up苔蔓扩张->m_wp苔蔓.lock();
	CHECK_RET_VOID(ref苔蔓.m_up苔蔓);
	;
	if (!ref苔蔓.m_up苔蔓->扩张结束())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "请先等此太岁的苔蔓(wàn)扩张到最大范围");
		return;
	}

	//必须在本苔蔓范围内
	if (!ref苔蔓.m_up苔蔓->在半径内(refPos))
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "请放置在选中太岁的苔蔓(wàn)上");
		return;
	}

	m_wp分裂 = 造建筑Component::创建建筑(m_refEntity.m_refSpace, refPos, 太岁, m_refEntity.m_upPlayer, EntitySystem::GetNickName(m_refEntity));
	CHECK_WP_RET_VOID(m_wp分裂);
}

太岁Component::太岁Component(Entity& ref) :m_refEntity(ref)
{

}
