#include "pch.h"
#include "AoiComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "Space.h"

void AoiComponent::Add(Space& refSpace, Entity& refEntity)
{
	refEntity.m_upAoi.reset(new AoiComponent(refSpace, refEntity));// std::make_unique<AoiComponent>();
	for (auto [k, sp] : refSpace.m_mapEntity)
	{
		if (EntitySystem::Is�ӿ�(refEntity))
		{
			if (sp->m_upAoi && refEntity.m_upAoi)//����ģ�ͣ��໥��ס
			{
				sp->m_upAoi->m_map�ܿ����ҵ�[refEntity.Id] = refEntity.weak_from_this();
				refEntity.m_upAoi->m_map���ܿ�����[sp->Id] = sp;
			}
		}

		if (EntitySystem::Is�ӿ�(*sp))
		{

			if (!sp->m_upAoi)
			{
				LOG(ERROR) << "";
				continue;
			}

			refEntity.m_upAoi->m_map�ܿ����ҵ�[sp->Id] = sp;
			sp->m_upAoi->m_map���ܿ�����[refEntity.Id] = refEntity.weak_from_this();
		}
	}
}

void AoiComponent::OnDestory()
{
	for (auto [k, wp] : m_map���ܿ�����) 
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map�ܿ����ҵ�.erase(m_refEntity.Id);
	}
	m_map���ܿ�����.clear();

	for (auto [k, wp] : m_map�ܿ����ҵ�)
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map���ܿ�����.erase(m_refEntity.Id);
	}
	m_map�ܿ����ҵ�.clear();
}

AoiComponent::AoiComponent(Space& refSpace, Entity& refEntity) :m_refSpace(refSpace), m_refEntity(refEntity)
{
}
