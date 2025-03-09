#pragma once
#include "../CoRoutine/CoDb.h"
class 慢操作AliyunGreen
{
public:
	/// <summary>
	/// 可在主线程协程里调用，实际只把请求放进队列，然后什么也不做
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiterBool& CoAliyunGreen(const std::string& strContent, FunCancel& cancel);
	慢操作<bool> m_慢操作;

private:
};


