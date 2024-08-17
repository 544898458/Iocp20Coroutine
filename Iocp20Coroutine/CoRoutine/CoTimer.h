#pragma once
#include<stdint.h>
#include<chrono>
#include <functional>
#include"CoTask.h"
namespace CoTimer
{
	CoAwaiterBool& Wait(const std::chrono::milliseconds &, FunCancel& cancel);
	CoAwaiterBool& WaitNextUpdate(FunCancel& cancel);
	void Update();
};

