#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;
class PlayerComponent;
class Entity;

namespace ���˾���
{
	CoTask<int> Coѵ��ս(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Coѵ��ս_��(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co����ս(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co����ս(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
};

