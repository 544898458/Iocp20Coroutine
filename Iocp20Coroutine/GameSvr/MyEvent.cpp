#include "pch.h"
#include "MyEvent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "单位组件/PlayerComponent.h"

bool MyEvent::SameSpace(const WpEntity& refWpEntity, const Space& refSpace, const std::string& stdNickName)
{
	auto sp = refWpEntity.lock();
	if (EntitySystem::GetNickName(*sp) != stdNickName)
		return false;

	return &sp->m_refSpace == &refSpace;
}
