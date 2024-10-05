#include "pch.h"
#include "CoLock.h"
#include <map>
#include <deque>
#include <tuple>
std::map<std::string, std::deque<std::tuple<CoAwaiterBool,FunRunCurrentCo>>> g_mapLock;
FunRunCurrentCo g_funRunCurrentCo;

CoLock::~CoLock()
{
	auto iter = g_mapLock.find(m_strLockKey);
	auto& refDeque = iter->second;
	auto& [_, fun] = refDeque.back();
	fun();
	refDeque.pop_back();
}

CoAwaiterBool& CoLock::Lock(const std::string& strLockKey, FunCancel &funCancel)
{
	m_strLockKey = strLockKey;
	auto& refDeque = g_mapLock[strLockKey];
	refDeque.push_front({ CoAwaiterBool(1, funCancel) , g_funRunCurrentCo});
	std::tuple<CoAwaiterBool, FunRunCurrentCo> &refTuple = refDeque.front();
	auto &[refAwaiter, _] = refTuple;
	refAwaiter.m_bAwaitReady = refDeque.size() == 1;
	refAwaiter.SetResult(false);
	return refAwaiter;
	
}
