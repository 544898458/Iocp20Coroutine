#include "pch.h"
#include "BuffComponent.h"
#include "DefenceComponent.h"
#include "��Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../RecastNavigationCrowd.h"

BuffComponent::BuffComponent(Entity& ref) :m_refEntity(ref)
{
}

CoTaskBool BuffComponent::Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯)
{
	while (!co_await CoTimer::Wait(dura���, m_funCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_spDefence);
		if (m_refEntity.IsDead())
			co_return false;

		if (m_refEntity.m_spDefence->����Ѫ())
			continue;

		m_refEntity.m_spDefence->m_hp += i16�仯;
		m_refEntity.BroadcastNotifyPos();
	}
	co_return false;
}

BuffComponent& BuffComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_upBuff.reset(new BuffComponent(refEntity));
	return *refEntity.m_upBuff;
}

void BuffComponent::������(uint32_t u32Buff��Id, ��λ�������� ��������, float f�仯, std::chrono::system_clock::duration duraɾ��)
{
	auto& refMap���� = m_map������ֵ[��������];
	auto iterOld���� = refMap����.find(u32Buff��Id);
	float fOld�仯 = 0;
	if (iterOld���� != refMap����.end())
	{
		fOld�仯 = iterOld����->second.�仯;
		refMap����.erase(iterOld����);
	}
	auto [pair, ok] = refMap����.insert({ u32Buff��Id, {f�仯} });
	CHECK_RET_VOID(ok);
	[��������, u32Buff��Id, &refMap����, this, duraɾ��](FunCancel& fun)->CoTaskBool
		{
			const auto local�������� = ��������;
			const auto u32Buff��IdLocal = u32Buff��Id;
			auto& refEntity = *this;
			auto& refMap����Local = refMap����;
			if (co_await CoTimer::Wait(duraɾ��, fun))
				co_return true;

			const auto sizeErase = refMap����Local.erase(u32Buff��IdLocal);//��������Э�̣��������ֻ�ܵ��þֲ�����
			CHECK_CO_RET_FALSE(sizeErase == 1);
			refEntity.On���Ա仯(local��������);
			co_return true;
		}(pair->second.funCancel).RunNew();

		if (f�仯 != fOld�仯)
		{
			On���Ա仯(��������);
		}
}

void BuffComponent::On���Ա仯(��λ�������� ��������)
{
	if (�������� == ��λ��������::�ƶ��ٶ�)
	{
		CHECK_RET_VOID(m_refEntity.m_sp��);
		if (!m_refEntity.m_sp��->m_wpRecastNavigationCrowd.expired())
			m_refEntity.m_sp��->m_wpRecastNavigationCrowd.lock()->SetSpeed();
	}
}
float BuffComponent::����(��λ�������� ����) const
{
	const auto iterFind = m_map������ֵ.find(����);
	if (iterFind == m_map������ֵ.end())
		return 0;

	float ����ֵ = 0;
	for (const auto& [_, ������ֵ] : iterFind->second)
	{
		����ֵ += ������ֵ.�仯;
	}
	return ����ֵ;
}

void BuffComponent::��ʱ��Ѫ()
{
	using namespace std;
	Co��ʱ����ֵ(2s, 1).RunNew();
}

void BuffComponent::OnDestroy()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

