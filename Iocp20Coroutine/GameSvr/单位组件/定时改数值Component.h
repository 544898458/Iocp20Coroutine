#pragma once
#include "../../CoRoutine/CoTask.h"

class Entity;
class PlayerGateSession_Game;
class Space;

class 定时改数值Component
{
public:
	定时改数值Component(Entity &ref);
	static void AddComponent(Entity& refEntity);
	void OnDestroy();
private:
	CoTaskBool Co定时改数值(std::chrono::system_clock::duration dura间隔, int16_t i16变化);
	Entity& m_refEntity;
	FunCancel m_funCancel;
};

