#include "pch.h"
#include "数值Component.h"
#include "Entity.h"


数值Component::数值Component(Entity& refEntity) : m_refEntity(refEntity)
{
}

数值Component& 数值Component::AddComponent(Entity& refEntity)
{
	CHECK_RET_VALUE(!refEntity.m_up数值, *refEntity.m_up数值);
	refEntity.m_up数值.reset(new 数值Component(refEntity));
	return *refEntity.m_up数值;
}

int 数值Component::Get(const Entity& refEntity, const 属性类型 属性)
{
	if(!refEntity.m_up数值)
		return 0;

	return refEntity.m_up数值->Get(属性);
}

int 数值Component::Set(Entity& refEntity, const 属性类型 属性, const int 数值)
{
	CHECK_RET_DEFAULT(refEntity.m_up数值);
	return refEntity.m_up数值->Set(属性, 数值);
}

int 数值Component::改变(Entity& refEntity, const 属性类型 属性, const int 变化)
{
	CHECK_RET_DEFAULT(refEntity.m_up数值);
	return refEntity.m_up数值->改变(属性, 变化);
}

int 数值Component::Get(const 属性类型 属性) const
{
	const auto iterFind = m_map属性数值.find(属性);
	if (m_map属性数值.end() == iterFind)
		return 0;

	return iterFind->second;
}

int 数值Component::Set(const 属性类型 属性, const int 数值)
{
	auto& ref数值 = m_map属性数值[属性];
	ref数值 = 数值;
	return ref数值;
}

int 数值Component::改变(const 属性类型 属性, const int 变化)
{
	auto& ref数值 = m_map属性数值[属性];
	ref数值 += 变化;
	return ref数值;
}
