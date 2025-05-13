#include "pch.h"
#include "CoTimer.h"
#include <map>
/// <summary>
/// 协程调度，定时器Awaiter
/// </summary>
namespace CoTimer
{
	//CoAwaiterBool Wait2()
	//{
	//	return CoAwaiterBool();
	//}

	std::multimap<std::chrono::steady_clock::time_point, CoAwaiterBool > g_multiTimer;
	std::map<long, CoAwaiterBool> g_NextUpdate;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="milli"></param>
	/// <param name="cancel"></param>
	/// <returns>true表示中途取消</returns>
	CoAwaiterBool& Wait(const std::chrono::system_clock::duration& milli, FunCancel& cancel, const std::string& strDebugInfo)
	{
		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		const auto time = std::chrono::steady_clock::now() + milli;
		const auto sn = CoAwaiterBool::GenSn();
		auto iter = g_multiTimer.insert({ time ,CoAwaiterBool(sn, cancel, strDebugInfo + "," + __FUNCTION__) });
		
		cancel = [time, sn]()
			{
				//LOG(INFO) << "Wait取消" << sn;
				auto pair = g_multiTimer.equal_range(time);
				for (auto iter = pair.first; iter != pair.second; )
				{
					if (iter->second.Sn() != sn)
					{
						++iter;
						continue;
					}

					iter->second.Run(true);//迭代器失效
					//break;
					iter = g_multiTimer.erase(iter);
				}
			};
		return iter->second;
	}
	/// <summary>
	/// 
	/// </summary>
	/// <param name="cancel"></param>
	/// <returns>true表示中途取消</returns>
	CoAwaiterBool& WaitNextUpdate(FunCancel& cancel, const std::string& strDebugInfo)
	{

		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		const auto sn = CoAwaiterBool::GenSn();
		auto pair = g_NextUpdate.insert({ sn,CoAwaiterBool(sn, cancel, strDebugInfo + "," + __FUNCTION__) });
		
		cancel = [sn]()
			{
				auto snLocal = sn;
				//LOG(INFO) << "WaitNextUpdate取消" << sn;
				auto itFind = g_NextUpdate.find(snLocal);
				if (itFind == g_NextUpdate.end())
				{
					LOG(ERROR) << "";
					_ASSERT(false);
					return;
				}
				itFind->second.Run(true);
				const auto count删除 = g_NextUpdate.erase(snLocal);
				_ASSERT(count删除 == 1);
				//cancel = old;
			};
		return pair.first->second;
	}
	void Update()
	{
		std::vector<int> vecDel;
		for (auto& kv : g_NextUpdate)
		{
			vecDel.push_back(kv.second.Sn());
		}
		for (const auto sn : vecDel)
		{
			auto itFind = g_NextUpdate.find(sn);
			if (itFind == g_NextUpdate.end())
			{
				//_ASSERT(false);
				//LOG(WARING) << "N";	下面的Run里可能删除g_NextUpdate.erase(sn);
				continue;
			}
			itFind->second.Run(false);
			g_NextUpdate.erase(sn);
		}

		const auto now = std::chrono::steady_clock::now();
		while (!g_multiTimer.empty())
		{
			auto iter = g_multiTimer.begin();
			auto& kv = *iter;
			if (kv.first > now)
				return;

			kv.second.Run(false);
			//_ASSERT(kv.second.Finished());
			g_multiTimer.erase(kv.first);
		}
	}
	void OnAppExit()
	{
	}
}