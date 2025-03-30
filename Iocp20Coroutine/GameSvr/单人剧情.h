#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;
class PlayerComponent;
class Entity;

namespace 单人剧情
{
	CoTask<int> Co训练战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co训练战_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co防守战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co攻坚战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
};

