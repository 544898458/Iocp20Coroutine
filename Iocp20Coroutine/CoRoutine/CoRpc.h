#pragma once
#include <functional>
#include <map>
#include "CoTask.h"

namespace CoRpc
{
	extern int g_rpcSnId;
	extern std::map<int, CoAwaiter> g_mapRpc;
	extern std::map<int, FunCancel> g_mapRpcCancel;

	template<class T_Req, class T_Responce>
	CoAwaiter& Send(T_Req&& req, const std::function<void(const T_Req&)>& funSend)
	{
		++g_rpcSnId;
		req.rpcSnId = g_rpcSnId;
		funSend(req);
		auto iterCancel = g_mapRpcCancel.insert({ g_rpcSnId, FunCancel()});
		auto iter = g_mapRpc.insert({ g_rpcSnId, CoAwaiter(true, iterCancel.first->second) });
		return iter.first->second;
	}

};