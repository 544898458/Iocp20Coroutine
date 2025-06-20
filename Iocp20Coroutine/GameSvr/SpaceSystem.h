#pragma once
#include "SpEntity.h"
#include "单位组件/找目标走过去Component.h"

class Space;
struct Position;
enum 单位类型;
namespace SpaceSystem
{
	std::vector<SpEntity> AddMonster(Space& refSpace, const 单位类型 类型, const Position& refPos, const int count = 1, 找目标走过去Component::Fun空闲走向目标 fun空闲走向目标 = 找目标走过去Component::怪物闲逛);
};

