#include "pch.h"
#include "��ʱ����ֵComponent.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"

��ʱ����ֵComponent::��ʱ����ֵComponent(Entity& ref) :m_refEntity(ref)
{
}

CoTaskBool ��ʱ����ֵComponent::Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯)
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

��ʱ����ֵComponent& ��ʱ����ֵComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_up��ʱ����ֵ.reset(new ��ʱ����ֵComponent(refEntity));
	return *refEntity.m_up��ʱ����ֵ;
}

void ��ʱ����ֵComponent::������(��λ�������� ��������, int16_t �仯, std::chrono::system_clock::duration duraɾ��)
{
	auto& refMap���� = m_map������ֵ[��������];
	auto [pair, ok] = refMap����.insert({ ++m_sn������ֵ, {�仯} });
	CHECK_RET_VOID(ok);
	[&refMap����, this, duraɾ��](FunCancel& fun)->CoTaskBool
		{
			auto sn = m_sn������ֵ;
			auto& refMap����Local = refMap����;
			if (co_await CoTimer::Wait(duraɾ��, fun))
				co_return false;

			const auto sizeErase = refMap����Local.erase(sn);
			CHECK_CO_RET_FALSE(sizeErase == 1);
		}(pair->second.funCancel).RunNew();
}

int16_t ��ʱ����ֵComponent::����(��λ�������� ����) const
{
	const auto iterFind = m_map������ֵ.find(����);
    if (iterFind == m_map������ֵ.end())
		return 0;

	int16_t ����ֵ = 0;
	for (const auto& [_, ������ֵ] : iterFind->second)
	{
		����ֵ += ������ֵ.�仯;
	}
	return ����ֵ;
}

void ��ʱ����ֵComponent::��ʱ��Ѫ()
{
	using namespace std;
	Co��ʱ����ֵ(2s, 1).RunNew();
}

void ��ʱ����ֵComponent::OnDestroy()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

