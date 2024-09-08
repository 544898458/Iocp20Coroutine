#pragma once
#include "CoTask.h"
#include <deque>
template<class T>
class CoDb
{
public:
	/// <summary>
	/// 可在主线程协程里调用，实际只把请求放进队列，然后什么也不做
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiterBool& Save(const T& ref, FunCancel& cancel);
	/// <summary>
	/// 独立线程调用，很耗时的操作，真的操作数据库，也可能是读写文件，也可能是调用网上的云数据库的接口
	/// </summary>
	void DbThreadProcess();
	/// <summary>
	/// 主线程（单线程）调用，得到数据库执行结果（执行协程下一句）
	/// </summary>
	void Process();
	std::deque<std::tuple<T, CoAwaiterBool>> m_dequeSave;
	std::deque<CoAwaiterBool> m_dequeResult;
};

