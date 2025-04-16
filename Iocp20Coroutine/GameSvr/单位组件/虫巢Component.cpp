#pragma once
#include "pch.h"
#include "�泲Component.h"
#include "����λComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"

inline �泲Component::�泲Component(Entity& ref) :m_refEntity(ref), m_pos�׳漯���(ref.Pos())
{
	Co���׳�().RunNew();
}

void �泲Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�泲 = std::make_shared<�泲Component, Entity&>(refEntity);
}

void �泲Component::TryCancel()
{
	if (m_Cancel���׳�)
		m_Cancel���׳�();
}

void �泲Component::OnLoad()
{
}

void �泲Component::Set�����(const Position& pos)
{
	m_pos�׳漯��� = pos;
	for (auto wp : m_listWp�׳�)
	{
		if (wp.expired())
			continue;

		if (wp.lock()->m_sp����λ)
			wp.lock()->m_sp����λ->m_pos����� = pos;
	}
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
CoTaskBool �泲Component::Co���׳�()
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
		auto wp = m_refEntity.m_refSpace.����λ(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, �׳�);
		CHECK_WP_CONTINUE(wp);
		m_listWp�׳�.emplace_back(wp);
		auto& ref = *wp.lock();
		if(ref.m_sp����λ)
			ref.m_sp����λ->m_pos����� = m_pos�׳漯���;

	}

	co_return false;
}
