#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
struct Position;
class Space;
class GameSvrSession;
struct MsgChangeMoneyResponce;
class PlayerGateSession_Game;
namespace AiCo
{
	//其它的同步函数都可以直接传 Entity&
	//协程中的全局对象也可以直接穿引用，不用智能指针
	//如果是Entity自己的协程，可以用引用Entity&，前提是确保Entity销毁前停止这个协程
	CoTask<int> Attack(SpEntity spEntity, WpEntity spDefencer, FunCancel& cancel);
	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel);
	CoTaskBool WalkToPos(Entity& refThis, const Position &posTarget, FunCancel& funCancel, const float f距离目标小于此距离停下 = 0);
	CoTaskBool WalkToTarget(Entity &refThis, SpEntity spEntity, FunCancel& funCancel, const bool b检查警戒距离 = true);
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel);
	CoTask<std::tuple<bool, MsgChangeMoneyResponce>> ChangeMoney(PlayerGateSession_Game& refSession, int32_t changeMoney, bool addMoney, FunCancel& funCancel);
	CoTask<int> AddMoney(PlayerGateSession_Game& refSession, FunCancel& funCancel);
};

