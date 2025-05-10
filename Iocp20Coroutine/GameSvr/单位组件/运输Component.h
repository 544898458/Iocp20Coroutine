#pragma once
#include "SpEntity.h"
#include <list>
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"

class Entity;

class 运输Component
{
public:
    运输Component(Entity& ref);
    static void AddComponent(Entity& refEntity);
    void OnEntityDestroy(const bool bDestroy);
    void OnLoad();
    
    // 运输相关功能
    bool 可以运输(const Entity& refEntity) const;
    bool 开始运输(SpEntity spEntity);
    void 卸载单位();
    uint16_t 获取运输单位数量() const { return (uint16_t)m_list运输单位.size(); }
    uint16_t 获取最大运输数量() const { return 8; } // 房虫可以运输8个单位

private:
    Entity& m_refEntity;
    std::list<SpEntity> m_list运输单位;
    FunCancel m_TaskCancel;
}; 