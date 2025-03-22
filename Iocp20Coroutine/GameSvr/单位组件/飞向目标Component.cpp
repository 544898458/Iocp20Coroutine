#include "pch.h"
#include "飞向目标Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"

void 飞向目标Component::AddComponet(Entity& refEntity, const Position& pos目标)
{
	refEntity.m_up飞向目标.reset(new 飞向目标Component(refEntity, pos目标));
}

飞向目标Component::飞向目标Component(Entity& ref, const Position& pos目标) :m_refEntity(ref), m_pos目标(pos目标)
{
	m_TaskCancel.co = Co飞向目标遇敌爆炸();
}

CoTaskBool 飞向目标Component::Co飞向目标遇敌爆炸()
{
	while (!co_await CoTimer::WaitNextUpdate(m_TaskCancel.cancel))
	{
		if (m_refEntity.Pos().DistanceLessEqual(m_pos目标, 1))
		{
			using namespace std;
			m_refEntity.CoDelayDelete(1ms).RunNew();
			co_return false;
		}
		const auto vec方向 = (m_pos目标 - m_refEntity.Pos()).归一化();
		单位::战斗配置 配置;
		CHECK_CO_RET_FALSE(单位::Find战斗配置(m_refEntity.m_类型, 配置));
		m_refEntity.SetPos(m_refEntity.Pos() + vec方向 * 配置.f每帧移动距离);
		m_refEntity.BroadcastNotifyPos();
	}
	co_return true;
}