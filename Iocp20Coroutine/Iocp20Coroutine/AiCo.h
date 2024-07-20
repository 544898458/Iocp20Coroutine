#pragma once
#include "../CoRoutine/CoTask.h"
class Entity;
class MyServer;
namespace AiCo
{
	CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, std::function<void()>& cancel);
	CoTask<int> Idle(Entity* pEntity, float& x, float& z, std::function<void()>& funCancel);
	CoTask<int> WalkToPos(Entity* pEntity, float& x, float& z, const float targetX, const float targetZ, MyServer *pServer, std::function<void()>& funCancel);
	CoTask<int> WalkToTarget(Entity* pThis, Entity* pEntity, MyServer* pServer, std::function<void()>& funCancel);
};

