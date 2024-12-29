#include "pch.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "Space.h"
#include "��Component.h"
#include "AttackComponent.h"
#include "PlayerNickNameComponent.h"

void EntitySystem::BroadcastEntity����(Entity& refEntity, const std::string& refStrGbk)
{
	refEntity.Broadcast<MsgEntity����>({ .idEntity = refEntity.Id, .str���� = StrConv::GbkToUtf8(refStrGbk) });
}

void EntitySystem::BroadcastChangeSkeleAnimIdle(Entity& refEntity)
{
	refEntity.BroadcastChangeSkeleAnim("idle");
}

void EntitySystem::Broadcast��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı�)
{
	refEntity.Broadcast<Msg��������>({ .str���� = StrConv::GbkToUtf8(refStr����), .str�ı� = StrConv::GbkToUtf8(str�ı�) });
}

bool EntitySystem::Is�ӿ�(const Entity& refEntity)
{
	if (refEntity.m_spDefence)
		return false;

	if (!refEntity.m_spPlayer)
		return false;

	assert(refEntity.m_upAoi);
	if (!refEntity.m_upAoi)
		return false;

	return true;
}

bool EntitySystem::�����ѷ���λ̫��(Entity& refEntity)
{
	const auto wp��������ڹ������ѷ���λ = refEntity.m_refSpace.Get�����Entity(refEntity, false,
		[](const Entity& ref)->bool
		{
			if (nullptr == ref.m_spDefence)
				return false;

			if (!ref.m_spAttack)
				return false;
			
			return (bool)ref.m_spAttack->m_cancelAttack;
		});
	bool b�����ѷ���λ̫�� = false;
	if (!wp��������ڹ������ѷ���λ.expired())
	{
		b�����ѷ���λ̫�� = refEntity.DistanceLessEqual(*wp��������ڹ������ѷ���λ.lock(), 3);
	}
	return b�����ѷ���λ̫��;
}

const std::string EntitySystem::GetNickName(Entity& refEntity)
{
	if (refEntity.m_spPlayerNickName)
		return refEntity.m_spPlayerNickName->m_strNickName;

	return {};
}
