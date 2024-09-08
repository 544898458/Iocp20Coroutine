#include "pch.h"
#include <glog/logging.h>
#include <cstdlib>
#include "MyMsgQueue.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "../IocpNetwork/MsgQueueTemplate.h"
#include "../IocpNetwork/StrConv.h"
#include "AiCo.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "../CoRoutine/CoRpc.h"

MsgNotifyPos::MsgNotifyPos(Entity& ref) : entityId((uint64_t)&ref), x(ref.m_Pos.x), z(ref.m_Pos.z), eulerAnglesY(ref.m_eulerAnglesY), hp(ref.m_hp)
{}