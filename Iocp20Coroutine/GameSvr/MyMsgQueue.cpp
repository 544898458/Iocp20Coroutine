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
#include "单位组件/PlayerComponent.h"
#include "../CoRoutine/CoRpc.h"
#include "单位组件/DefenceComponent.h"


/// <summary>
/// 平方就是2次方
/// </summary>
const float fExponent = 2.0f;

MsgNotifyPos::MsgNotifyPos(Entity& ref) : entityId(ref.Id), x(ref.Pos().x), z(ref.Pos().z), eulerAnglesY(ref.m_eulerAnglesY)
{
	if (ref.m_spDefence)
		hp = ref.m_spDefence->m_hp;
}

MsgAddRoleRet::MsgAddRoleRet(Entity& ref) :
	entityId(ref.Id),
	nickName(StrConv::GbkToUtf8(ref.头顶Name())),
	entityName(StrConv::GbkToUtf8(ref.m_配置.strName)),
	prefabName(StrConv::GbkToUtf8(ref.m_配置.strPrefabName)),
	i32HpMax(ref.m_spDefence ? ref.m_spDefence->m_i32HpMax : 0),
	类型(ref.m_类型)
{
}

Position Position::operator*(float f)const
{
	return { x * f,z * f };
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

float Position::LengthPow2()const
{
	return std::pow(x, fExponent) + std::pow(z, fExponent);
}

float Position::Length()const
{
	return std::sqrtf(LengthPow2());
}
/// <summary>
/// 向量归一化(Vector Normalization)
/// </summary>

Position Position::归一化()const
{
	const float f范数 = std::sqrtf(std::pow(x, fExponent) + std::pow(z, fExponent));
	if (0 == f范数)
	{
		LOG(ERROR) << "范数是0，无法归一化," << *this;
		return { x ,z };
	}

	return { x / f范数, z / f范数 };
}
