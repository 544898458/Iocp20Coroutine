#include "CoTimer.h"
#include <map>
namespace CoTimer
{
	CoAwaiter Wait2()
	{
		return CoAwaiter();
	}

	std::multimap<std::chrono::steady_clock::time_point, CoTask<int> > g_multiTimer;
	std::vector<CoAwaiter> g_NextUpdate;
	CoAwaiter Wait(const std::chrono::milliseconds& milli)
	{
		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		return CoAwaiter();
	}
	CoAwaiter& WaitNextUpdate()
	{
		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		g_NextUpdate.push_back(CoAwaiter());
		return g_NextUpdate.back();
	}
	void Update()
	{
		auto old = g_NextUpdate;
		g_NextUpdate.clear();
		for (auto& awaiter : old)
		{
			awaiter.m_hAwaiter.resume();
		}
		

		const auto now = std::chrono::steady_clock::now();
		while (!g_multiTimer.empty())
		{
			auto& kv = *g_multiTimer.begin();
			if (kv.first < now)
				return;

			kv.second.Run();
			assert(kv.second.Finished());
			g_multiTimer.erase(kv.first);
		}
	}
}