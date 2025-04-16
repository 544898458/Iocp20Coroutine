#pragma once
#include "../../CoRoutine/CoTask.h"

class Entity;
class PlayerGateSession_Game;
class Space;

class ��ʱ����ֵComponent
{
public:
	��ʱ����ֵComponent(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void OnDestroy();
private:
	CoTaskBool Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯);
	Entity& m_refEntity;
	FunCancel m_funCancel;
};

