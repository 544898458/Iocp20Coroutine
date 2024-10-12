#include "pch.h"
#include "MyEvent.h"
#include "Entity.h"

bool MyEvent::SameSpace(const WpEntity& refWpEntity, const Space& refSpace)
{
	return &refWpEntity.lock()->m_refSpace == &refSpace;
}
