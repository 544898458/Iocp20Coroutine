#include "pch.h"
#include "MyEvent.h"
#include "Entity.h"
#include "PlayerComponent.h"

bool MyEvent::SameSpace(const WpEntity& refWpEntity, const Space& refSpace, PlayerGateSession_Game& refGateSession)
{
	auto sp = refWpEntity.lock();
	if (!sp->m_spPlayer || &sp->m_spPlayer->m_refSession != &refGateSession)
		return false;

	return &sp->m_refSpace == &refSpace;
}
