#pragma once
#include<stdint.h>
#include<chrono>
#include"CoRoutine/CoTask.h"
namespace CoTimer
{
	CoTask<int> Wait(const std::chrono::milliseconds &);
	void Update();
};

