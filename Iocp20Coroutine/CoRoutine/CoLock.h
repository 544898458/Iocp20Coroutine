#pragma once
#include <string>
#include "CoTask.h"
/// <summary>
/// CoLock lock;
/// 
/// </summary>
class CoLock final
{
public:
	CoLock()
	{

	}
	~CoLock() 
	{

	}
	CoAwaiterBool& Lock(const std::string& strLockKey, FunCancel& funCancel);
};

