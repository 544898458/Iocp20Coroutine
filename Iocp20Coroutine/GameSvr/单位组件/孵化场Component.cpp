#pragma once
#include "pch.h"
#include "������Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"

inline ������Component::������Component(Entity& ref) :m_refEntity(ref)
{
	Co���׳�().RunNew();
}

void ������Component::AddComponet(Entity& refEntity)
{
	refEntity.m_sp������ = std::make_shared<������Component, Entity&>(refEntity);
}

void ������Component::TryCancel()
{
	if (m_Cancel���׳�)
		m_Cancel���׳�();
}

void ������Component::OnLoad()
{
}

int Rand(int min, int maxValid)
{
	if (min > maxValid)
	{
		_ASSERT(false);
		LOG(ERROR) << "Rand:" << min << " ����<= " << maxValid;
		return min;
	}
	return min + 1 + std::rand() % (maxValid - min);
}
CoTaskBool ������Component::Co���׳�()
{
	KeepCancel kc(m_Cancel���׳�);
	using namespace std;
	while (!co_await CoTimer::Wait(10s, m_Cancel���׳�))
	{
		std::erase_if(m_listWp�׳�, [](const auto& wp)->bool {return wp.expired(); });
		if (m_listWp�׳�.size() >= 3)
			continue;

		auto pos = m_refEntity.Pos();
		pos.x += Rand(-10, 10);
		m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
		m_listWp�׳�.emplace_back(m_refEntity.m_refSpace.����λ(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, �׳�));
	}

	co_return false;
}
