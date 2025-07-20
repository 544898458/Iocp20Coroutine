#pragma once
#include "SpEntity.h"

class Entity;

class 跟随Component
{
public:
    跟随Component(Entity& refEntity);
    static void AddComponent(Entity& refEntity);
    void 跟随(WpEntity& wpEntity);
    void 取消跟随();
    void OnEntityDestroy(const bool bDestroy);

private:
    CoTaskBool Co顶层设置空闲走向目标();
    Entity& m_refEntity;
    WpEntity m_wp目标;
    FunCancel m_funCancel;
}; 