#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;


namespace 单人剧情
{
	CoTask<int> Co训练战(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
	CoTask<int> Co防守战(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
};

