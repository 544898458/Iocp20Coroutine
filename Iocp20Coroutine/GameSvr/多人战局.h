#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;
class PlayerComponent;
class Entity;

namespace ����ս��
{
	CoTask<int> Co�ķ���ս(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
};

