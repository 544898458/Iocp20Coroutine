#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class GameSvr;
struct Position;
class Space;
namespace AiCo
{
	//协程不会同步执行结束，所以Entity全都要用SpEntity，不能用引用，实参一进去立刻加引用计数
	//其它的同步函数都可以直接传 Entity&
	//协程中的全局对象也可以直接穿引用，不用智能指针
	CoTask<int> Attack(SpEntity spEntity, SpEntity spDefencer, FunCancel& cancel);
	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel);
	CoTask<int> WalkToPos(SpEntity spThis, const Position &posTarget, FunCancel& funCancel);
	CoTask<int> WalkToTarget(SpEntity spThis, SpEntity spEntity, GameSvr* pServer, FunCancel& funCancel);
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel);
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel);
	CoTask<int> AddMoney(SpEntity spThis, FunCancel& funCancel);
};

