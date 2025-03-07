#pragma once
#include "pch.h"
#include "孵化场Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"

inline 孵化场Component::孵化场Component(Entity& ref) :m_refEntity(ref)
{
	Co造幼虫().RunNew();
}

void 孵化场Component::AddComponet(Entity& refEntity)
{
	refEntity.m_sp孵化场 = std::make_shared<孵化场Component, Entity&>(refEntity);
}

void 孵化场Component::TryCancel()
{
	if (m_Cancel造幼虫)
		m_Cancel造幼虫();
}

void 孵化场Component::OnLoad()
{
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
CoTaskBool 孵化场Component::Co造幼虫()
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
		m_listWp幼虫.emplace_back(m_refEntity.m_refSpace.造活动单位(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, 幼虫));
	}

	co_return false;
}
