#pragma once
#include <functional>
#include <vector>
#include "CoTask.h"
#include <tuple>

/// <summary>
/// 要求在主线程单线程中调用
/// </summary>
template<class T_Responce>
class CoEvent
{
public:
	using CoAwaiterRet = CoAwaiter<std::tuple<bool, T_Responce> >;
	using FunWant = std::function<bool(const T_Responce&)>;

	static void OnRecvEvent(const T_Responce& responce)
	{
		//auto iterFindCancel = g_mapRpcCancel.find(req.rpcSnId);
		std::vector<uint32_t> vec;
		for (auto& [k, tuple] : g_mapEvent)
		{
			vec.emplace_back(k);
		}

		for (auto k : vec)
		{
			auto iterFind = g_mapEvent.find(k);
			if (iterFind == g_mapEvent.end())
			{
				LOG(ERROR) << "";
				_ASSERT(false);
				continue;
			}

			auto& [coAwaiter, fun] = iterFind->second;
			if (fun(responce))
			{
				auto ret = std::make_tuple(false, responce);
				coAwaiter.Run(ret);//此处可能加g_mapRpc
				g_mapEvent.erase(iterFind);
			}
		}
	}

	static CoAwaiter<std::tuple<bool, T_Responce>>& Wait(FunCancel& funCancel, const std::string strDebugInfo, FunWant funWantEvent = [](const T_Responce&) {return true; })
	{
		++g_rpcSnId;
		const uint32_t rpcSnId = g_rpcSnId;
		auto iter = g_mapEvent.insert(std::make_pair<uint32_t, std::tuple<CoAwaiterRet, FunWant >>((uint32_t)rpcSnId, { CoAwaiterRet(rpcSnId, funCancel, strDebugInfo), funWantEvent }));

		funCancel = [rpcSnId]()
			{
				//OnRecvEvent(true, {});
				auto iterFind = g_mapEvent.find(rpcSnId);
				if (iterFind == g_mapEvent.end())
				{
					LOG(ERROR) << "";
					_ASSERT(false);
					return;
				}

				auto& [coAwaiter, fun] = iterFind->second;
				auto ret = std::make_tuple(true, T_Responce());
				coAwaiter.Run(ret);
				g_mapEvent.erase(iterFind);
			};
		auto& [coAwaiter, fun] = iter.first->second;
		return coAwaiter;
	}

private:
	static uint32_t g_rpcSnId;
	static std::map<uint32_t, std::tuple<CoAwaiterRet, FunWant >> g_mapEvent;
	//static std::map<uint32_t, FunCancel&> g_mapRpcCancel;
};

template<class T_Responce>	uint32_t CoEvent<T_Responce>::g_rpcSnId = 0;
template<class T_Responce>	std::map<uint32_t, std::tuple<CoAwaiter<std::tuple<bool, T_Responce> >, std::function<bool(const T_Responce&)>>> CoEvent<T_Responce>::g_mapEvent;
//template<class T_Responce>	std::map<int, FunCancel> CoRpc<T_Responce>::g_mapRpcCancel;
