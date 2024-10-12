#pragma once
#include "SpEntity.h"
class Space;

namespace MyEvent
{
	struct MoveEntity
	{
		WpEntity wpEntity;
	};
	bool SameSpace(const WpEntity& refWpEntity, const Space& refSpace);;
}

