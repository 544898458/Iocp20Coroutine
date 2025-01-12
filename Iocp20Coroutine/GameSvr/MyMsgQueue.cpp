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


MsgNotifyPos::MsgNotifyPos(Entity& ref) : entityId(ref.Id), x(ref.Pos().x), z(ref.Pos().z), eulerAnglesY(ref.m_eulerAnglesY)
{
	if (ref.m_spDefence)
		hp = ref.m_spDefence->m_hp;
}

MsgAddRoleRet::MsgAddRoleRet(Entity& ref) :
	entityId(ref.Id),
	nickName(StrConv::GbkToUtf8(ref.NickName())),
	entityName(StrConv::GbkToUtf8(ref.m_配置.strName)),
	prefabName(StrConv::GbkToUtf8(ref.m_配置.strPrefabName)),
	i32HpMax(ref.m_spDefence ? ref.m_spDefence->m_i32HpMax : 0)
{
}