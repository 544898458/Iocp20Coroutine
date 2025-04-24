#pragma once
#include<stdint.h>
#include<chrono>
#include <functional>
#include"CoTask.h"
namespace CoTimer
{
	CoAwaiterBool& Wait(const std::chrono::system_clock::duration&, FunCancel& cancel, const std::string& strDebugInfo = "");
	CoAwaiterBool& WaitNextUpdate(FunCancel& cancel, const std::string& strDebugInfo = "");
	void Update();
	void OnAppExit();
};

