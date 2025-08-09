#include "pch.h"
#include "巡逻Component.h"
#include "找目标走过去Component.h"
#include "走Component.h"
#include "../Entity.h"
#include "../EntitySystem.h"
#include "../Space.h"
#include "../../CoRoutine/CoTimer.h"

巡逻Component::巡逻Component(Entity& refEntity)
    : m_refEntity(refEntity)
{
}

void 巡逻Component::AddComponent(Entity& refEntity)
{
    CHECK_RET_VOID(!refEntity.m_up巡逻);
    refEntity.AddComponentOnDestroy(&Entity::m_up巡逻, refEntity);
}

void 巡逻Component::开始巡逻(const std::vector<Position>& vec巡逻点, bool b循环)
{
    m_vec巡逻点 = vec巡逻点;
    m_idx当前 = 0;
    m_b循环 = b循环;
    m_b启用 = !m_vec巡逻点.empty();
    if (m_b启用)
    {
        Co顶层设置空闲走向巡逻点().RunNew();
        EntitySystem::BroadcastEntity描述(m_refEntity, "开始巡逻");
    }
}

void 巡逻Component::取消巡逻()
{
    m_b启用 = false;
    if (m_funCancel)
    {
        m_funCancel();
        m_funCancel = nullptr;
    }
    if (m_refEntity.m_up找目标走过去)
    {
        // 清除空闲移动回调，避免继续空闲走动
        m_refEntity.m_up找目标走过去->m_fun空闲走向此处 = nullptr;
    }
}

CoTaskBool 巡逻Component::Co顶层设置空闲走向巡逻点()
{
    using namespace std::chrono_literals;
    while (!co_await CoTimer::Wait(1s, m_funCancel))
    {
        if (!m_b启用)
            continue;

        if (!m_refEntity.m_up找目标走过去)
            continue;

        if (m_vec巡逻点.empty())
            continue;

        if (走Component::正在走(m_refEntity))
            continue;

        if (m_refEntity.m_up采集 && !m_refEntity.m_up采集->m_TaskCancel.co.Finished())
            continue;

        CHECK_CO_RET_FALSE(m_idx当前 < m_vec巡逻点.size());
        auto posTarget = m_vec巡逻点[m_idx当前];
        m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);

        if (m_refEntity.Pos().DistanceLessEqual(posTarget, 2.0f))// 如果已经靠近当前点，则切到下一个点
        {
            if (m_idx当前 + 1 < m_vec巡逻点.size())
            {
                ++m_idx当前;
            }
            else
            {
                m_idx当前 = 0;
                m_vec巡逻点.reverse();
            }

            continue;
        }

        if (!m_refEntity.m_up找目标走过去->m_fun空闲走向此处)
        {
            m_refEntity.m_up找目标走过去->m_fun空闲走向此处 = [posTarget](const Position& /*pos*/)
            {
                return posTarget;
            };
        }
    }
    
    co_return true;
}

void 巡逻Component::OnEntityDestroy(const bool /*bDestroy*/)
{
    取消巡逻();
} 