#pragma once
class Entity;
class RecastNavigationCrowd final
{
public:
	RecastNavigationCrowd(Entity& refEntity, const Position& posTarget);
	~RecastNavigationCrowd();
	void SetMoveTarget(const Position& posTarget);
	void SetSpeed();
	Entity& m_refEntity;
};