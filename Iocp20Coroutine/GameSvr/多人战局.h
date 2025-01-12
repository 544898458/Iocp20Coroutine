#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;
class PlayerComponent;
class Entity;

namespace 多人战局
{
	CoTask<int> Co四方对战(Space& refSpace, Entity& ref视口, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
};

