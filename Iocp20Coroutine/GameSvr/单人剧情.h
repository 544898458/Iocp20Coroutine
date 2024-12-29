#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;
class PlayerComponent;
class Entity;

namespace 单人剧情
{
	CoTask<int> Co训练战(Space& refSpace, FunCancel& funCancel, std::shared_ptr<PlayerComponent> spPlayer可能空, const std::string strNickName);
	CoTask<int> Co防守战(Space& refSpace, Entity& ref视口, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
};

