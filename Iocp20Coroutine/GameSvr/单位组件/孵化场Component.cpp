#pragma once
#include "pch.h"
#include "單陴Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"

inline 單陴Component::單陴Component(Entity& ref) :m_refEntity(ref)
{
	Co婖衿單().RunNew();
}

void 單陴Component::AddComponet(Entity& refEntity)
{
	refEntity.m_sp單陴 = std::make_shared<單陴Component, Entity&>(refEntity);
}

void 單陴Component::TryCancel()
{
	if (m_Cancel婖衿單)
		m_Cancel婖衿單();
}

void 單陴Component::OnLoad()
{
}

int Rand(int min, int maxValid)
{
	if (min > maxValid)
	{
		_ASSERT(false);
		LOG(ERROR) << "Rand:" << min << " 斛剕<= " << maxValid;
		return min;
	}
	return min + 1 + std::rand() % (maxValid - min);
}
CoTaskBool 單陴Component::Co婖衿單()
{
	KeepCancel kc(m_Cancel婖衿單);
	using namespace std;
	while (!co_await CoTimer::Wait(10s, m_Cancel婖衿單))
	{
		std::erase_if(m_listWp衿單, [](const auto& wp)->bool {return wp.expired(); });
		if (m_listWp衿單.size() >= 3)
			continue;

		auto pos = m_refEntity.Pos();
		pos.x += Rand(-10, 10);
		m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
		m_listWp衿單.emplace_back(m_refEntity.m_refSpace.婖魂雄等弇(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, 衿單));
	}

	co_return false;
}
