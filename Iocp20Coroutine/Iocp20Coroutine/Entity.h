#pragma once
#include "CoRoutine/CoTask.h"
#include "MsgQueue.h"
#include <functional>

class Space;
class Entity
{
public:
	Entity();
	void Init(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun);
	void ReplaceCo(std::function<CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun);
	void Update();
	void Hurt(int) {}
	Position m_Pos;
	CoTask<int> m_coWalk;
	CoTask<int> m_coAttack;
	//std::function<void()> m_cancelAttack;
	std::function<void()> m_cancel;
	const uint64_t Id;
	int m_hp = 10;
	std::string m_nickName;
	MySession* m_pSession = nullptr;
private:
	Space *m_space;
	
};
