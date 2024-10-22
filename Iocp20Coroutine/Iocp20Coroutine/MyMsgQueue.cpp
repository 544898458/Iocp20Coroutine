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
#include "PlayerComponent.h"
#include "../CoRoutine/CoRpc.h"
#include "DefenceComponent.h"

MsgNotifyPos::MsgNotifyPos(Entity& ref) : entityId(ref.Id), x(ref.m_Pos.x), z(ref.m_Pos.z), eulerAnglesY(ref.m_eulerAnglesY)
{
	if (ref.m_spDefence)
		hp = ref.m_spDefence->m_hp;
}

MsgAddRoleRet::MsgAddRoleRet(Entity& ref) :entityId(ref.Id), nickName(StrConv::GbkToUtf8(ref.NickName())), entityName(StrConv::GbkToUtf8(ref.m_strEntityName)), prefabName(ref.m_strPrefabName)
{
}