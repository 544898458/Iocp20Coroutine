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
		if (EntitySystem::Is视口(refEntity))
		{
			if (sp->m_upAoi && refEntity.m_upAoi)//灯塔模型，相互记住
			{
				sp->m_upAoi->m_map能看到我的[refEntity.Id] = refEntity.weak_from_this();
				refEntity.m_upAoi->m_map我能看到的[sp->Id] = sp;
			}
		}

		if (EntitySystem::Is视口(*sp))
		{

			if (!sp->m_upAoi)
			{
				LOG(ERROR) << "";
				continue;
			}

			refEntity.m_upAoi->m_map能看到我的[sp->Id] = sp;
			sp->m_upAoi->m_map我能看到的[refEntity.Id] = refEntity.weak_from_this();
		}
	}
}

void AoiComponent::OnDestory()
{
	for (auto [k, wp] : m_map我能看到的) 
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map能看到我的.erase(m_refEntity.Id);
	}
	m_map我能看到的.clear();

	for (auto [k, wp] : m_map能看到我的)
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map我能看到的.erase(m_refEntity.Id);
	}
	m_map能看到我的.clear();
}

AoiComponent::AoiComponent(Space& refSpace, Entity& refEntity) :m_refSpace(refSpace), m_refEntity(refEntity)
{
}
