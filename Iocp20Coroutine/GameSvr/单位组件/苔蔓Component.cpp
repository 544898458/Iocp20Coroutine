#include "pch.h"
#include "Ì¦ÂûComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"

void Ì¦ÂûComponent::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_upÌ¦Âû) << "m_upÌ¦Âû";
	refEntity.m_upÌ¦Âû.reset(new Ì¦ÂûComponent(refEntity));
}

void Ì¦ÂûComponent::TryCancel()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

Ì¦ÂûComponent::Ì¦ÂûComponent(Entity& ref):m_refEntity(ref)
{
	if (!ref.m_spAttack)
	{
		LOG(ERROR) << "m_spAttack";
		return;
	}
	CoÌ¦ÂûÏûÍö().RunNew();
}

CoTaskBool Ì¦ÂûComponent::CoÌ¦ÂûÏûÍö()
{
	using namespace std;
	while (!co_await CoTimer::Wait(20s, m_funCancel))
	{
		if (m_wp¸½×Å½¨Öş.expired())
			continue;

		m_i16°ë¾¶ += 2;
		if (m_i16°ë¾¶ < 0)
		{
			m_refEntity.DelayDelete();
			co_return false;
		}
	}
	co_return false;
}
