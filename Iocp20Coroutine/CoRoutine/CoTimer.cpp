#include "CoTimer.h"
#include <map>
/// <summary>
/// 协程调度，定时器Awaiter
/// </summary>
namespace CoTimer
{
	//CoAwaiter Wait2()
	//{
	//	return CoAwaiter();
	//}

	std::multimap<std::chrono::steady_clock::time_point, CoAwaiter > g_multiTimer;
	std::map<long, CoAwaiter> g_NextUpdate;
	CoAwaiter& Wait(const std::chrono::milliseconds& milli, FunCancel& cancel)
	{
		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		const auto time = std::chrono::steady_clock::now() + milli;
		auto iter = g_multiTimer.insert({ time ,CoAwaiter(true, cancel) });
		const auto sn = iter->second.Sn();
		//FunCancel old = cancel;
		cancel = [time, sn]()
			{
				LOG(INFO) << "Wait取消" << sn;
				auto pair = g_multiTimer.equal_range(time);
				for (auto iter = pair.first; iter != pair.second; ++iter)
				{
					if (iter->second.Sn() == sn)
					{
						iter->second.Cancel();//迭代器失效
						break;
						//g_multiTimer.erase(iter);
					}
				}
				//再次回复迭代器删除
				pair = g_multiTimer.equal_range(time);
				for (auto iter = pair.first; iter != pair.second; ++iter)
				{
					if (iter->second.Sn() == sn)
					{
						g_multiTimer.erase(iter);//迭代器失效
						break;	
					}
				}
				//cancel = old;
			};
		return iter->second;
	}
	CoAwaiter& WaitNextUpdate(FunCancel &cancel)
	{

		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		auto ret = CoAwaiter(true, cancel);
		g_NextUpdate[ret.Sn()] = ret;
		const auto sn = ret.Sn();
		//FunCancel old = cancel;
		cancel = [sn]()
			{
				//LOG(INFO) << "WaitNextUpdate取消" << sn;
				g_NextUpdate[sn].Cancel();
				g_NextUpdate.erase(sn);
				//cancel = old;
			};
		return g_NextUpdate[ret.Sn()];
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
			g_NextUpdate[sn].Run();
			g_NextUpdate.erase(sn);
		}
		
		const auto now = std::chrono::steady_clock::now();
		while (!g_multiTimer.empty())
		{
			auto iter = g_multiTimer.begin();
			auto& kv = *iter;
			if (kv.first > now)
				return;

			kv.second.Run();
			//assert(kv.second.Finished());
			g_multiTimer.erase(kv.first);
		}
	}
}