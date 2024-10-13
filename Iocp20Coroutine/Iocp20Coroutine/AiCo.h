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
	//协程不会同步执行结束，所以Entity全都要用SpEntity，不能用引用，实参一进去立刻加引用计数
	//其它的同步函数都可以直接传 Entity&
	//协程中的全局对象也可以直接穿引用，不用智能指针
	//如果是Entity自己的协程，可以用引用Entity&，前提是确保Entity销毁前停止这个协程
	CoTask<int> Attack(SpEntity spEntity, SpEntity spDefencer, FunCancel& cancel);
	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel);
	CoTaskBool WalkToPos(SpEntity spThis, const Position &posTarget, FunCancel& funCancel);
	CoTaskBool WalkToTarget(SpEntity spThis, SpEntity spEntity, FunCancel& funCancel);
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel);
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel);
	CoTask<std::tuple<bool, MsgChangeMoneyResponce>> ChangeMoney(PlayerGateSession_Game& refSession, uint32_t changeMoney, bool addMoney, FunCancel& funCancel);
	CoTask<int> AddMoney(PlayerGateSession_Game& refSession, FunCancel& funCancel);
};

