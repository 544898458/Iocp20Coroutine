#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;
class PlayerComponent;
class Entity;

namespace ���˾���
{
	CoTask<int> Coѵ��ս(Space& refSpace, FunCancel& funCancel, std::shared_ptr<PlayerComponent> spPlayer���ܿ�, const std::string strNickName);
	CoTask<int> Co����ս(Space& refSpace, Entity& ref�ӿ�, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
};

