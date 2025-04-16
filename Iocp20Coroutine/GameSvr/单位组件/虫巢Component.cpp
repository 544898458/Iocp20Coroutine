#pragma once
#include "pch.h"
#include "虫巢Component.h"
#include "造活动单位Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"

inline 虫巢Component::虫巢Component(Entity& ref) :m_refEntity(ref), m_pos幼虫集结点(ref.Pos())
{
	Co造幼虫().RunNew();
}

void 虫巢Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp虫巢 = std::make_shared<虫巢Component, Entity&>(refEntity);
}

void 虫巢Component::TryCancel()
{
	if (m_Cancel造幼虫)
		m_Cancel造幼虫();
}

void 虫巢Component::OnLoad()
{
}

void 虫巢Component::Set集结点(const Position& pos)
{
	m_pos幼虫集结点 = pos;
	for (auto wp : m_listWp幼虫)
	{
		if (wp.expired())
			continue;

		if (wp.lock()->m_sp造活动单位)
			wp.lock()->m_sp造活动单位->m_pos集结点 = pos;
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
		std::erase_if(m_listWp幼虫, [](const auto& wp)->bool {return wp.expired(); });
		if (m_listWp幼虫.size() >= 3)
			continue;

		auto pos = m_refEntity.Pos();
		pos.x += Rand(-10, 10);
		m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
		auto wp = m_refEntity.m_refSpace.造活动单位(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, 幼虫);
		CHECK_WP_CONTINUE(wp);
		m_listWp幼虫.emplace_back(wp);
		auto& ref = *wp.lock();
		if(ref.m_sp造活动单位)
			ref.m_sp造活动单位->m_pos集结点 = m_pos幼虫集结点;

	}

	co_return false;
}
