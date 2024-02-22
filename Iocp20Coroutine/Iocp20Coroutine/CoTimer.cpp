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
	std::map<long,CoAwaiter> g_NextUpdate;
	CoAwaiter& Wait(const std::chrono::milliseconds& milli, std::function<void()> &cancel)
	{
		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		const auto time = std::chrono::steady_clock::now() + milli;
		auto iter = g_multiTimer.insert({ time ,CoAwaiter(true) });
		const auto sn = iter->second.Sn();
		cancel = [time,sn]()
			{
				auto pair = g_multiTimer.equal_range(time);
				for (auto iter = pair.first; iter != pair.second; ++iter)
				{
					if (iter->second.Sn() == sn)
					{
						iter->second.m_hAwaiter.resume();
						g_multiTimer.erase(iter);
					}
				}
			};
		return iter->second;
	}
	CoAwaiter& WaitNextUpdate(std::function<void()>& cancel)
	{

		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		auto ret = CoAwaiter(true);
		g_NextUpdate[ret.Sn()] = ret;
		const auto sn = ret.Sn();
		auto old = cancel;
		cancel = [sn,&cancel,old]()
			{
				g_NextUpdate[sn].Cancel();
				g_NextUpdate.erase(sn);
				cancel = old;
			};
		return g_NextUpdate[ret.Sn()];
	}
	void Update()
	{
		auto old = g_NextUpdate;
		g_NextUpdate.clear();
		for (auto& awaiter : old)
		{
			awaiter.second.m_hAwaiter.resume();
		}
		

		const auto now = std::chrono::steady_clock::now();
		while (!g_multiTimer.empty())
		{
			auto iter = g_multiTimer.begin();
			auto& kv = *iter;
			if (kv.first > now)
				return;

			if( ! kv.second.m_hAwaiter.done() )
			kv.second.m_hAwaiter.resume();
			//assert(kv.second.Finished());
			g_multiTimer.erase(kv.first);
		}
	}
}