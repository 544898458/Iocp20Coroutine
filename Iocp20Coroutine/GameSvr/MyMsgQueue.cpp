#include "pch.h"
#include <glog/logging.h>
#include <cstdlib>
#include "MyMsgQueue.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueTemplate.h"
#include "../IocpNetwork/StrConv.h"
#include "AiCo.h"
#include "Entity.h"
#include "��λ���/PlayerComponent.h"
#include "../CoRoutine/CoRpc.h"
#include "��λ���/DefenceComponent.h"


/// <summary>
/// ƽ������2�η�
/// </summary>
const float fExponent = 2.0f;

MsgNotifyPos::MsgNotifyPos(Entity& ref) : entityId(ref.Id), x(ref.Pos().x), z(ref.Pos().z), eulerAnglesY(ref.m_eulerAnglesY)
{
	if (ref.m_spDefence)
		hp = ref.m_spDefence->m_hp;
}

MsgAddRoleRet::MsgAddRoleRet(Entity& ref) :
	entityId(ref.Id),
	nickName(StrConv::GbkToUtf8(ref.ͷ��Name())),
	entityName(StrConv::GbkToUtf8(ref.m_����.strName)),
	prefabName(StrConv::GbkToUtf8(ref.m_����.strPrefabName)),
	i32HpMax(ref.m_spDefence ? ref.m_spDefence->m_i32HpMax : 0),
	����(ref.m_����)
{
}

bool Position::DistanceLessEqual(const Position& refPos, float fDistance) const
{
	return this->DistancePow2(refPos) <= std::pow(fDistance, fExponent);
}

float Position::DistancePow2(const Position& refPos) const
{
	return std::pow(x - refPos.x, fExponent) + std::pow(z - refPos.z, fExponent);
}

float Position::Distance(const Position& refPos) const
{
	return std::sqrtf(DistancePow2(refPos));
}
