#pragma once
#include "pch.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
#include "../SpPlayerComponent.h"

#include "虫巢Component.h"
#include "造活动单位Component.h"
#include "BuildingComponent.h"

虫巢Component::虫巢Component(Entity& ref) :m_refEntity(ref), m_Cancel造幼虫("m_Cancel造幼虫")
{
	m_mapPos幼虫集结点[单位类型::单位类型_Invalid_0] = ref.Pos();
	Co造幼虫().RunNew();
}

void 虫巢Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up虫巢, refEntity);
}

void 虫巢Component::OnEntityDestroy(const bool bDestroy)
{
	m_Cancel造幼虫.TryCancel();
}

void 虫巢Component::OnLoad()
{
}

void 虫巢Component::Set集结点(const Position& pos, const 单位类型 类型)
{
	m_mapPos幼虫集结点[类型] = pos;
	for (auto wp : m_listWp幼虫)
	{
		if (wp.expired())
			continue;

		if (wp.lock()->m_up造活动单位)
			wp.lock()->m_up造活动单位->m_mapPos集结点[类型] = pos;
	}
}

int Rand(int min, int maxValid)
{
	if (min > maxValid)
	{
		_ASSERT(false);
		LOG(ERROR) << "Rand:" << min << " 必须<= " << maxValid;
		return min;
	}
	return min + 1 + std::rand() % (maxValid - min);
}
CoTaskBool 虫巢Component::Co造幼虫()
{
	KeepCancel kc(m_Cancel造幼虫);
	using namespace std;
	while (!co_await CoTimer::Wait(10s, m_Cancel造幼虫))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_upBuilding);
		if (!m_refEntity.m_upBuilding->已造好())
			continue;

		std::erase_if(m_listWp幼虫, [](const auto& wp)->bool {return wp.expired(); });
		if (m_listWp幼虫.size() >= 3)
			continue;

		auto pos = m_refEntity.Pos();
		pos.x += Rand(-10, 10);
		m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
		auto wp = m_refEntity.m_refSpace.造活动单位(std::forward<UpPlayerComponent>(m_refEntity.m_upPlayer), EntitySystem::GetNickName(m_refEntity), pos, 幼虫);
		CHECK_WP_CONTINUE(wp);
		m_listWp幼虫.emplace_back(wp);
		auto& ref = *wp.lock();
		if(ref.m_up造活动单位)
			ref.m_up造活动单位->m_mapPos集结点 = m_mapPos幼虫集结点;

	}

	co_return false;
}
