#include "pch.h"
#include "Ì¦ÂûÀ©ÕÅComponent.h"
#include "Ì¦ÂûComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"

void Ì¦ÂûÀ©ÕÅComponent::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_upÌ¦ÂûÀ©ÕÅ) << "m_upm_upÌ¦ÂûÀ©ÕÅ";
	refEntity.m_upÌ¦ÂûÀ©ÕÅ.reset(new Ì¦ÂûÀ©ÕÅComponent(refEntity));
}

Ì¦ÂûÀ©ÕÅComponent::Ì¦ÂûÀ©ÕÅComponent(Entity& ref) :m_refEntity(ref)
{
	Co·´¸´¼Ó³ÖĞøµôÑªBuff().RunNew();
}

CoTaskBool Ì¦ÂûÀ©ÕÅComponent::Co·´¸´¼Ó³ÖĞøµôÑªBuff()
{
	using namespace std;
	while (!co_await CoTimer::Wait(5s, m_funCancel))
	{
		CHECK_WP_CO_RET_FALSE(m_wpÌ¦Âû);
		auto& refÌ¦Âû = *m_wpÌ¦Âû.lock();

		CHECK_CO_RET_FALSE(refÌ¦Âû.m_upÌ¦Âû);

		refÌ¦Âû.m_upÌ¦Âû->m_i16°ë¾¶ += 2;
		EntitySystem::BroadcastEntityÌ¦Âû°ë¾¶(refÌ¦Âû);

		if (refÌ¦Âû.m_upÌ¦Âû->m_i16°ë¾¶ > Ì¦ÂûComponent::MAX°ë¾¶)
		{
			co_return false;
		}
	}
	co_return false;
}

void Ì¦ÂûÀ©ÕÅComponent::TryCancel()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}
