#include "pch.h"
#include "运输Component.h"
#include "../Entity.h"
#include "../EntitySystem.h"
#include "../Space.h"
#include "../../CoRoutine/CoTimer.h"
#include "PlayerComponent.h"
#include "走Component.h"
#include "BuildingComponent.h"

运输Component::运输Component(Entity& ref) : m_refEntity(ref)
{
}

void 运输Component::AddComponent(Entity& refEntity)
{
    refEntity.AddComponentOnDestroy(&Entity::m_up运输, new 运输Component(refEntity));
}

void 运输Component::OnEntityDestroy(const bool bDestroy)
{
    if (bDestroy)
    {
        m_TaskCancel.TryCancel();
        // 卸载所有运输的单位
        卸载单位();
    }
}

void 运输Component::OnLoad()
{
}

bool 运输Component::可以运输(const Entity& refEntity) const
{
    // 检查是否是房虫
    if (m_refEntity.m_类型 != 房虫)
        return false;

    // 检查目标单位是否可以被运输
    if (refEntity.m_类型 == 房虫 || refEntity.m_类型 == 虫巢 || refEntity.m_类型 == 基地)
        return false;

    // 检查运输单位数量是否达到上限
    if (获取运输单位数量() >= 获取最大运输数量())
        return false;

    // 检查目标单位是否在房虫附近
    if (!m_refEntity.DistanceLessEqual(refEntity, 2.0f))
        return false;

    return true;
}

bool 运输Component::开始运输(SpEntity spEntity)
{
    if (!可以运输(*spEntity))
        return false;

    // 将单位添加到运输列表
    m_list运输单位.push_back(spEntity);
    
    // 设置单位的所有者为房虫
    spEntity->m_wpOwner = m_refEntity.weak_from_this();
    
    // 从空间中移除单位
    m_refEntity.m_refSpace.RemoveEntity(spEntity);
    
    // 通知玩家
    if (m_refEntity.m_upPlayer)
    {
        PlayerComponent::Say系统(m_refEntity, std::format("已装载单位，当前运输数量：{}/{}", 获取运输单位数量(), 获取最大运输数量()));
    }

    return true;
}

void 运输Component::卸载单位()
{
    auto list = m_list运输单位;
    m_list运输单位.clear();
    
    for (auto& sp : list)
    {
        if (sp.expired())
            continue;
            
        // 将单位添加回空间
        m_refEntity.m_refSpace.AddEntity(sp);
        
        // 设置单位位置在房虫附近
        auto pos = m_refEntity.Pos();
        pos.x += std::rand() % 5 - 2;
        pos.z += std::rand() % 5 - 2;
        m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
        sp->SetPos(pos);
        
        // 清除单位的所有者
        sp->m_wpOwner.reset();
        
        // 广播单位进入
        sp->BroadcastEnter();
    }
    
    // 通知玩家
    if (m_refEntity.m_upPlayer)
    {
        PlayerComponent::Say系统(m_refEntity, "已卸载所有单位");
    }
} 