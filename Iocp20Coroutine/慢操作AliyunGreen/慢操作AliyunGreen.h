#pragma once
#include "../CoRoutine/CoDb.h"
enum 单位类型;
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
	CoAwaiterBool& Co记录战局结束(const uint32_t uSvrId, const std::string& nickName, int 战局类型, bool is赢, FunCancel& cancel);
	CoAwaiterBool& Co记录击杀(const uint32_t uSvrId, const int 战局类型, const std::string& nickName攻击, const 单位类型 单位类型攻击, const std::string& nickName阵亡, const 单位类型 单位类型阵亡, FunCancel& cancel);
	慢操作<bool> m_慢操作;

private:
};


