#include "pch.h"
#include "CoLock.h"
#include <map>
#include <deque>
#include <tuple>
std::map<std::string, std::deque<CoAwaiterBool>> g_mapLock;
//FunRunCurrentCo g_funRunCurrentCo;

CoLock::~CoLock()
{
	CHECK_RET_VOID(!g_mapLock.empty());
	_ASSERT(!g_mapLock.empty());
	auto iter = g_mapLock.find(m_strLockKey);
	auto& refDeque = iter->second;
	CHECK_RET_VOID(!refDeque.empty());
	_ASSERT(!refDeque.empty());
	refDeque.pop_back();
	if (!refDeque.empty())
	{
		auto& refAwaiter = refDeque.back();
		if (!refAwaiter.m_bAwaitReady)
		{
			refAwaiter.Run();//refFun();
		}
	}

}

CoAwaiterBool& CoLock::Lock(const std::string& strLockKey, FunCancel& funCancel)
{
	m_strLockKey = strLockKey;
	auto& refDeque = g_mapLock[strLockKey];

	refDeque.push_front(CoAwaiterBool(1, funCancel, strLockKey + "," + __FUNCTION__));
	auto& refAwaiter = refDeque.front();
	refAwaiter.m_bAwaitReady = refDeque.size() == 1;
	refAwaiter.SetResult(false);
	return refAwaiter;

}
