#pragma once
#include "CoRoutine/CoTask.h"
#include "MsgQueue.h"
#include <functional>

class Space;
class Entity
{
public:
	Entity(float x, Space& space, std::function< CoTask<int>(Entity*, float&, bool&)> fun);
	void ReplaceCo(std::function<CoTask<int>(Entity*, float&, bool&)> fun);
	void Update()
	{
		co.Run();
	}
	float x = 0;
	CoTask<int> co;
	bool coStop = false;

private:
	Space& space;
	//MySession* pSession;
};

