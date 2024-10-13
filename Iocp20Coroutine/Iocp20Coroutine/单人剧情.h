#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;


namespace µ¥ÈË¾çÇé
{
	CoTask<int> Co(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
};

