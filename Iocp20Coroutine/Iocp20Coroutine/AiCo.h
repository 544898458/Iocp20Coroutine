#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class MyServer;
struct Position;
namespace AiCo
{
	//协程不会同步执行结束，所以Entity全都要用SpEntity，不能用引用，实参一进去立刻加引用计数
	//其它的同步函数都可以直接传 Entity&
	CoTask<int> Attack(SpEntity spEntity, SpEntity spDefencer, std::function<void()>& cancel);
	CoTask<int> Idle(SpEntity spEntity, float& x, float& z, std::function<void()>& funCancel);
	CoTask<int> WalkToPos(SpEntity spThis, const Position &posTarget, MyServer *pServer, std::function<void()>& funCancel);
	CoTask<int> WalkToTarget(SpEntity spThis, SpEntity spEntity, MyServer* pServer, std::function<void()>& funCancel);
	CoTask<int> WaitDelete(SpEntity spThis, std::function<void()>& funCancel);
};

