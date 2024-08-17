#pragma once
#include<stdint.h>
#include<chrono>
#include <functional>
#include"CoTask.h"
namespace CoTimer
{
	CoAwaiter& Wait(const std::chrono::milliseconds &, FunCancel& cancel);
	CoAwaiter& WaitNextUpdate(FunCancel& cancel);
	void Update();
};

