#pragma once
#include<stdint.h>
#include<chrono>
#include"CoRoutine/CoTask.h"
namespace CoTimer
{
	CoAwaiter Wait(const std::chrono::milliseconds &);
	CoAwaiter& WaitNextUpdate();
	void Update();
};

