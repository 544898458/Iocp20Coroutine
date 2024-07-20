#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"

class Space;
class MySession;
class MyServer;

class Entity
{
public:
	Entity();
	void Init(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun, MySession* pSession);
	void WalkToPos(const float targetX, const float targetZ, MyServer* pServer);
	void Update();
	void TryCancel();
	void Hurt(int) {}
	bool DistanceLessEqual(Entity* pEntity, float fDistance);
	void OnDestroy();
	Position m_Pos;
	CoTask<int> m_coWalk;
	CoTask<int> m_coAttack;
	//std::function<void()> m_cancelAttack;
	std::function<void()> m_cancel;
	const uint64_t Id;
	int m_hp = 10;
	const float m_fAttackDistance = 5.0f;
	std::string m_nickName;
	MySession* m_pSession = nullptr;
private:
	Space *m_space;
	
};
