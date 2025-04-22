#include "pch.h"
#include "../ö��/BuffId.h"
#include "../ö��/��������.h"
#include "BuffComponent.h"
#include "DefenceComponent.h"
#include "��Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../RecastNavigationCrowd.h"

BuffComponent::BuffComponent(Entity& ref) :m_refEntity(ref)
{
}

CoTaskBool BuffComponent::Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯, FunCancel& funCancel, const uint64_t idAttacker)
{
	while (!co_await CoTimer::Wait(dura���, funCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_upDefence);
		if (m_refEntity.IsDead())
			co_return false;

		if (0 < i16�仯)
			m_refEntity.m_upDefence->��Ѫ(i16�仯);
		else if (0 > i16�仯)
			m_refEntity.m_upDefence->����(-i16�仯, idAttacker);
		else
			LOG(ERROR) << "BuffComponent::������ �仯ֵ����";

		m_refEntity.BroadcastNotifyPos();
	}
	co_return false;
}

BuffComponent& BuffComponent::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_upBuff, new BuffComponent(refEntity));
	return *refEntity.m_upBuff;
}

void BuffComponent::������(BuffId idBuff��)
{
	��λ::Buff���� buff����;
	CHECK_RET_VOID(��λ::FindBuff����(idBuff��, buff����));
	auto& refMap���� = m_map������ֵ[buff����.����];
	auto iterOld���� = refMap����.find(idBuff��);
	float fOld�仯 = 0;
	if (iterOld���� != refMap����.end())
	{
		fOld�仯 = iterOld����->second.�仯;
		refMap����.erase(iterOld����);
	}
	auto [pair, ok] = refMap����.insert({ idBuff��, {buff����.f�仯ֵ} });
	CHECK_RET_VOID(ok);
	[buff����, idBuff��, &refMap����, this](FunCancel& fun)->CoTaskBool
		{
			const auto local�������� = buff����.����;
			const auto idBuff��Local = idBuff��;
			auto& refEntity = *this;
			auto& refMap����Local = refMap����;
			if (co_await CoTimer::Wait(buff����.dura���, fun))
				co_return true;

			const auto sizeErase = refMap����Local.erase(idBuff��Local);//��������Э�̣��������ֻ�ܵ��þֲ�����
			CHECK_CO_RET_FALSE(sizeErase == 1);
			refEntity.On���Ա仯(local��������);
			co_return true;
		}(pair->second.funCancel).RunNew();

		if (buff����.f�仯ֵ != fOld�仯)
		{
			On���Ա仯(buff����.����);
		}
}

void BuffComponent::On���Ա仯(�������� ��������)
{
	if (�������� == ��������::�ƶ��ٶ� && m_refEntity.m_up��)
	{
		if (!m_refEntity.m_up��->m_wpRecastNavigationCrowd.expired())
			m_refEntity.m_up��->m_wpRecastNavigationCrowd.lock()->SetSpeed();
	}
}
float BuffComponent::����(�������� ����) const
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

void BuffComponent::��ʱ����ֵ(const BuffId id, const uint64_t idAttacker)
{
	using namespace std;
	��λ::Buff���� buff����;
	CHECK_RET_VOID(��λ::FindBuff����(id, buff����));
	ɾBuff(id);
	Co��ʱ����ֵ(buff����.dura���, (int16_t)buff����.f�仯ֵ, m_mapFunCancel[id], idAttacker).RunNew();
}

void BuffComponent::ɾBuff(BuffId id)
{
	auto iterFind = m_mapFunCancel.find(id);
	if (iterFind != m_mapFunCancel.end() && iterFind->second)
	{
		iterFind->second();
		iterFind->second = nullptr;
	}
	m_mapFunCancel.erase(id);
}

bool BuffComponent::����Buff(BuffId id) const
{
	return m_mapFunCancel.find(id) != m_mapFunCancel.end();
}

void BuffComponent::OnEntityDestroy(const bool bDestroy)
{
	for (auto& [id, refFunCancel] : m_mapFunCancel)
	{
		if (refFunCancel)
		{
			refFunCancel();
			refFunCancel = nullptr;
		}
	}
	m_mapFunCancel.clear();

	m_map������ֵ.clear();
}

