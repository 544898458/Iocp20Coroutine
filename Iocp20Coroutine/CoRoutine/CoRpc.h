#pragma once
#include <functional>
#include <map>
#include "CoTask.h"

/// <summary>
/// 要求在主线程单线程中调用
/// </summary>
template<class T_Responce>
class CoRpc
{
public:
	static void OnRecvResponce(bool stop, const T_Responce& req)
	{
		g_mapRpcCancel.erase(req.rpcSnId);
		auto iterFind = g_mapRpc.find(req.rpcSnId);
		if (iterFind == g_mapRpc.end())
		{
			LOG(WARNING) << req.rpcSnId << "rpc收到回应时，协程已取消";
			//assert(false);
			return;
		}

		auto ret = std::make_tuple(stop, req);
		iterFind->second.Run(ret);
		g_mapRpc.erase(iterFind);
	}

	template<class T_Req>
	static CoAwaiter<std::tuple<bool, T_Responce>>& Send(T_Req&& req, const std::function<void(const T_Req&)>& funSend, FunCancel& funCancel)
	{
		++g_rpcSnId;
		req.rpcSnId = g_rpcSnId;
		funSend(req);
		auto iterCancel = g_mapRpcCancel.insert({ g_rpcSnId, funCancel });
		auto iter = g_mapRpc.insert({ g_rpcSnId, CoAwaiter< std::tuple<bool, T_Responce>>({true, iterCancel.first->second}) });
		const int rpcSnId = g_rpcSnId;
		funCancel = [rpcSnId]()
			{
				OnRecvResponce(true, { .rpcSnId = rpcSnId });
			};
		return iter.first->second;
	}

private:
	static int g_rpcSnId;
	static std::map<int, CoAwaiter<std::tuple<bool, T_Responce>>> g_mapRpc;
	static std::map<int, FunCancel&> g_mapRpcCancel;

};

template<class T_Responce>	int CoRpc<T_Responce>::g_rpcSnId = 0;
template<class T_Responce>	std::map<int, CoAwaiter<std::tuple<bool, T_Responce>>> CoRpc<T_Responce>::g_mapRpc;
template<class T_Responce>	std::map<int, FunCancel&> CoRpc<T_Responce>::g_mapRpcCancel;
