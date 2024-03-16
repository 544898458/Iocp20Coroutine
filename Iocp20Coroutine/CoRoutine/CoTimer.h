#pragma once
#include<stdint.h>
#include<chrono>
#include <functional>
#include"CoTask.h"
namespace CoTimer
{
	CoAwaiter& Wait(const std::chrono::milliseconds &, std::function<void()>& cancel);
	CoAwaiter& WaitNextUpdate(std::function<void()>& cancel);
	void Update();
};

