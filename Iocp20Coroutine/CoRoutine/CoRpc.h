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
	template<class T_Req>
	static CoAwaiter<T_Responce>& Send(T_Req&& req, const std::function<void(const T_Req&)>& funSend)
	{
		++g_rpcSnId;
		req.rpcSnId = g_rpcSnId;
		funSend(req);
		auto iterCancel = g_mapRpcCancel.insert({ g_rpcSnId, FunCancel() });
		auto iter = g_mapRpc.insert({ g_rpcSnId, CoAwaiter<T_Responce>(true, iterCancel.first->second) });
		return iter.first->second;
	}

	static void OnRecvResponce(const T_Responce& req)
	{
		g_mapRpcCancel.erase(req.rpcSnId);
		auto iterFind = g_mapRpc.find(req.rpcSnId);
		if (iterFind == g_mapRpc.end())
		{
			LOG(ERROR) << "ERR";
			assert(false);
			return;
		}

		iterFind->second.Run(req);
		g_mapRpc.erase(iterFind);
	}
private:
	static int g_rpcSnId;
	static std::map<int, CoAwaiter<T_Responce>> g_mapRpc;
	static std::map<int, FunCancel> g_mapRpcCancel;

};

template<class T_Responce>
int CoRpc<T_Responce>::g_rpcSnId = 0;
template<class T_Responce>
std::map<int, CoAwaiter<T_Responce>> CoRpc<T_Responce>::g_mapRpc;
template<class T_Responce>
std::map<int, FunCancel> CoRpc<T_Responce>::g_mapRpcCancel;
