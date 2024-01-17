#pragma once
#include "CoRoutine/CoTask.h"
#include "MsgQueue.h"
#include <functional>

class Space;
class Entity
{
public:
	Entity(float x,Space &space, std::function< CoTask<int>(float&)> fun);
	void Update()
	{
		co.Run();
	}
	float x = 0;
private:
	CoTask<int> co;
	Space& space;
	//MySession* pSession;
};

