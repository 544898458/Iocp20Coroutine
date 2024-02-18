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
	void Update()
	{
		m_coWalk.Run();
	}
	float x = 0;
	float z = 0;
	/// <summary>
	/// ��·Э�̣��滻���Э�̾�ִ���µ���·����
	/// </summary>
	CoTask<int> m_coWalk;
	/// <summary>
	/// �򵥵�Э���˳����ƣ��߼��ж�
	/// </summary>
	bool m_coStop = false;

private:
	Space& m_space;
};

