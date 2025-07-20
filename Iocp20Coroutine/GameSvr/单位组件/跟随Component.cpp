#include "pch.h"
#include "跟随Component.h"
#include "找目标走过去Component.h"
#include "Entity.h"
#include "../CoTimer.h"

跟随Component::跟随Component(Entity& refEntity)
    : m_refEntity(refEntity)
{
    
}
CoTaskBool 跟随Component::Co顶层设置空闲走向目标()
{
    while (! CoTimer::Wait(5s,m_funCancel))
    {
        if(m_wp目标.expired())
        {
            continue;
        }

        auto& refEntity = *m_wp目标.lock();
        if(refEntity.IsDead())
        {
            m_wp目标.reset();
            continue;
        }
        
        if(refEntity.m_up找目标走过去->m_fun空闲走向此处)
            continue;

        auto posTarget = refEntity.Pos();
        m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
        refEntity.m_up找目标走过去->m_fun空闲走向此处 = [this, posTarget](const Pos& pos)
        {
            return posTarget;
        };

    }
    co_return true;
}

void 跟随Component::AddComponent(Entity& refEntity)
{
    CHECK_RET_VOID(!refEntity.m_up跟随);
    refEntity.AddComponentOnDestroy(&Entity::m_up跟随, refEntity);
}

void 跟随Component::跟随(WpEntity& wpEntity)
{
    m_wp目标 = wpEntity;
    // 这里可以添加跟随逻辑
}

void 跟随Component::取消跟随()
{
    m_wp目标.reset();
}

void 跟随Component::OnEntityDestroy(const bool bDestroy)
{
    if(m_funCancel) 
    {
        m_funCancel();
        m_funCancel = nullptr;
    }
} 