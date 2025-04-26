#include "pch.h"
#include "��ֵComponent.h"
#include "Entity.h"
#include "../ö��/��������.h"


��ֵComponent::��ֵComponent(Entity& refEntity) : m_refEntity(refEntity)
{
}

��ֵComponent& ��ֵComponent::AddComponent(Entity& refEntity)
{
	CHECK_RET_VALUE(!refEntity.m_up��ֵ, *refEntity.m_up��ֵ);
	refEntity.m_up��ֵ.reset(new ��ֵComponent(refEntity));
	return *refEntity.m_up��ֵ;
}

int ��ֵComponent::Get(const Entity& refEntity, const �������� ����)
{
	if (!refEntity.m_up��ֵ)
		return 0;

	return refEntity.m_up��ֵ->Get(����);
}

int ��ֵComponent::Set(Entity& refEntity, const �������� ����, const int ��ֵ)
{
	CHECK_RET_DEFAULT(refEntity.m_up��ֵ);
	return refEntity.m_up��ֵ->Set(����, ��ֵ);
}

int ��ֵComponent::�ı�(Entity& refEntity, const �������� ����, const int �仯)
{
	CHECK_RET_DEFAULT(refEntity.m_up��ֵ);
	return refEntity.m_up��ֵ->�ı�(����, �仯);
}

int ��ֵComponent::Get(const �������� ����) const
{
	const auto iterFind = m_map������ֵ.find(����);
	if (m_map������ֵ.end() == iterFind)
		return 0;

	return iterFind->second;
}

int ��ֵComponent::Set(const �������� ����, const int ��ֵ)
{
	auto& ref��ֵ = m_map������ֵ[����];

	const auto old = ref��ֵ;
	ref��ֵ = ��ֵ;
	ȷ�������ڷ�Χ��(����);
	if (old != ref��ֵ)
		m_refEntity.BroadcastNotifyPos();

	return ref��ֵ;
}

int ��ֵComponent::�������(const ��������& ����)
{
	switch (����)
	{
	case ��������::����:
		return m_map������ֵ[�������];
		break;
	case ��������::����:
		return m_map������ֵ[�������];
		break;
	default:
		//LOG(ERROR) << "�������ʹ���" << ����;
		return std::numeric_limits<int>::max();
		break;
	}
}

int ��ֵComponent::�ı�(const �������� ����, const int �仯)
{
	auto& ref��ֵ = m_map������ֵ[����];
	const auto old = ref��ֵ;

	ref��ֵ += �仯;
	ȷ�������ڷ�Χ��(����);

	if (old != ref��ֵ)
		m_refEntity.BroadcastNotify����({����});

	return ref��ֵ;
}

int ��ֵComponent::ȷ�������ڷ�Χ��(const �������� ����)
{
	auto& ref��ֵ = m_map������ֵ[����];
	const auto max��ֵ = �������(����);
	ref��ֵ = std::max(0, std::min(ref��ֵ, max��ֵ));
	return ref��ֵ;
}
