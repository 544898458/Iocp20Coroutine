#pragma once
#include <functional>
#include <vector>
#include "CoTask.h"

/// <summary>
/// 要求在主线程单线程中调用
/// </summary>
template<class T_Responce>
class CoEvent
{
public:
	static void OnRecvEvent(bool stop, const T_Responce& responce)
	{
		//auto iterFindCancel = g_mapRpcCancel.find(req.rpcSnId);
		for (auto& [k, iterFind] : g_mapRpc)
		{
			auto ret = std::make_tuple(stop, responce);
			iterFind.Run(ret);
		}
		g_mapRpc.clear();
	}

	static CoAwaiter<std::tuple<bool, T_Responce>>& Wait(FunCancel& funCancel)
	{
		++g_rpcSnId;
			const uint32_t rpcSnId = g_rpcSnId;
		auto iter = g_mapRpc.insert({ g_rpcSnId, CoAwaiter< std::tuple<bool, T_Responce>>(rpcSnId, funCancel) });

		funCancel = [rpcSnId]()
			{
				OnRecvEvent(true, {});
			};
		return iter.first->second;
	}

private:
	static uint32_t g_rpcSnId;
	static std::map<int, CoAwaiter<std::tuple<bool, T_Responce>>> g_mapRpc;
	static std::map<int, FunCancel> g_mapRpcCancel;
};

template<class T_Responce>	uint32_t CoEvent<T_Responce>::g_rpcSnId = 0;
template<class T_Responce>	std::map<int, CoAwaiter<std::tuple<bool, T_Responce>>> CoEvent<T_Responce>::g_mapRpc;
//template<class T_Responce>	std::map<int, FunCancel> CoRpc<T_Responce>::g_mapRpcCancel;
