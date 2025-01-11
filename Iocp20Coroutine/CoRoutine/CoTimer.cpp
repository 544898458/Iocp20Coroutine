#include "CoTimer.h"
#include <map>
/// <summary>
/// Э�̵��ȣ���ʱ��Awaiter
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
	/// <returns>true��ʾ��;ȡ��</returns>
	CoAwaiterBool& Wait(const std::chrono::system_clock::duration& milli, FunCancel& cancel)
	{
		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		const auto time = std::chrono::steady_clock::now() + milli;
		const auto sn = CoAwaiterBool::GenSn();
		auto iter = g_multiTimer.insert({ time ,CoAwaiterBool(sn, cancel) });
		//FunCancel old = cancel;
		cancel = [time, sn]()
			{
				LOG(INFO) << "Waitȡ��" << sn;
				auto pair = g_multiTimer.equal_range(time);
				for (auto iter = pair.first; iter != pair.second; )
				{
					if (iter->second.Sn() != sn)
					{
						++iter;
						continue;
					}

					iter->second.Run(true);//������ʧЧ
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
	/// <returns>true��ʾ��;ȡ��</returns>
	CoAwaiterBool& WaitNextUpdate(FunCancel& cancel)
	{

		//g_multiTimer.insert({ std::chrono::steady_clock::now() + milli,Wait2() });
		//co_await(*g_multiTimer.begin()).second;
		//co_await Wait2();
		const auto sn = CoAwaiterBool::GenSn();
		auto pair = g_NextUpdate.insert({ sn,CoAwaiterBool(sn, cancel) });
		//FunCancel old = cancel;
		cancel = [sn]()
			{
				//LOG(INFO) << "WaitNextUpdateȡ��" << sn;
				auto itFind = g_NextUpdate.find(sn);
				if (itFind == g_NextUpdate.end())
				{
					assert(false);
					return;
				}
				itFind->second.Run(true);
				g_NextUpdate.erase(sn);
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
				//assert(false);
				//LOG(WARING) << "N";	�����Run�����ɾ��g_NextUpdate.erase(sn);
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
			//assert(kv.second.Finished());
			g_multiTimer.erase(kv.first);
		}
	}
	void OnAppExit()
	{
	}
}