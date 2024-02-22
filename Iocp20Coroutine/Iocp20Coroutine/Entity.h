#pragma once
#include "CoRoutine/CoTask.h"
#include "MsgQueue.h"
#include <functional>

class Space;
class Entity
{
public:
	Entity(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, bool&)> fun);
	void ReplaceCo(std::function<CoTask<int>(Entity*, float&, float&, bool&)> fun);
	void Update();
	float x = 0;
	float z = 0;
	/// <summary>
	/// 走路协程，替换这个协程就执行新的走路过程
	/// </summary>
	CoTask<int> m_coWalk;
	CoTask<int> m_coAttack;

	/// <summary>
	/// 简单的协程退出机制，逻辑判断
	/// </summary>
	bool m_coStop = false;
	const uint64_t Id;
	int m_hp = 10;
	std::string m_nickName;
private:
	Space& m_space;
	
};

