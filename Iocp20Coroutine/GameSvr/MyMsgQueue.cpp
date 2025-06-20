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
#include "../CoRoutine/CoRpc.h"
#include "枚举/属性类型.h"
#include "单位组件/PlayerComponent.h"
#include "单位组件/DefenceComponent.h"
#include "单位组件/苔蔓Component.h"
#include "单位组件/数值Component.h"

const std::initializer_list<const 属性类型> g_list所有属性 = { 生命, 能量 };
/// <summary>
/// 平方就是2次方
/// </summary>
const float fExponent = 2.0f;

MsgNotifyPos::MsgNotifyPos(const Entity& ref) :
	entityId(ref.Id), pos(ref.Pos()), eulerAnglesY(ref.m_eulerAnglesY)
{
}

MsgAddRoleRet::MsgAddRoleRet(Entity& ref) :
	entityId(ref.Id),
	nickName(StrConv::GbkToUtf8(ref.头顶Name())),
	entityName(StrConv::GbkToUtf8(ref.m_配置.strName)),
	prefabName(StrConv::GbkToUtf8(ref.m_配置.strPrefabName)),
	最大生命(数值Component::Get(ref, 属性类型::最大生命)),
	最大能量(数值Component::Get(ref, 属性类型::最大能量)),
	类型(ref.m_类型)
{
}

Position Position::operator*(float f)const
{
	return { x * f, 0, z * f };
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
		return { x , 0, z };
	}

	return { x / f范数, 0, z / f范数 };
}

Msg苔蔓半径::Msg苔蔓半径(Entity& refEntity) :idEntity(refEntity.Id)
{
	CHECK_RET_VOID(refEntity.m_up苔蔓);
	半径 = refEntity.m_up苔蔓->m_i16半径;
}

MsgNotify属性::MsgNotify属性(Entity& ref, std::initializer_list<const 属性类型> list) :idEntity(ref.Id)
{
	for (auto& 属性 : list)
		TryAdd属性(ref, 属性);
}

void MsgNotify属性::TryAdd属性(Entity& ref, const 属性类型 属性)
{
	const auto 值 = 数值Component::Get(ref, 属性);
	switch (属性)
	{
	case 属性类型::生命:
	case 属性类型::能量:
		break;
	default:
		if (0 >= 值)
			return;
		break;
	}

	map属性.insert({ 属性, (float)值 });
}
