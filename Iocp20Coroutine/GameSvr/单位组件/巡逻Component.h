#pragma once
#include <vector>
#include "../Position.h"
#include "SpEntity.h"
#include "../../CoRoutine/CoTask.h"

class Entity;

class 巡逻Component
{
public:
    巡逻Component(Entity& refEntity);
    static void AddComponent(Entity& refEntity);

    void 开始巡逻(const std::vector<Position>& vec巡逻点, bool b循环 = true);
    void 取消巡逻();
    void OnEntityDestroy(const bool bDestroy);

private:
    CoTaskBool Co顶层设置空闲走向巡逻点();

private:
    Entity& m_refEntity;
    std::vector<Position> m_vec巡逻点;
    uint16_t m_idx当前 = 0;
    FunCancel m_funCancel;
}; 