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
	void Hurt(int) {}
	Position m_Pos;
	/// <summary>
	/// ��·Э�̣��滻���Э�̾�ִ���µ���·����
	/// </summary>
	CoTask<int> m_coWalk;
	CoTask<int> m_coAttack;

	/// <summary>
	/// �򵥵�Э���˳����ƣ��߼��ж�
	/// </summary>
	bool m_coStop = false;
	const uint64_t Id;
	int m_hp = 10;
	std::string m_nickName;
private:
	Space& m_space;
	
};

CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, bool& stop);
