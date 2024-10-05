#include "pch.h"
#include "CoLock.h"
#include <map>
#include <deque>

std::map<std::string, std::deque<CoTask<int>*>> g_mapLock;
CoAwaiterBool& CoLock::Lock(const std::string& strLockKey, FunCancel &funCancel)
{
	auto itFind = g_mapLock.find(strLockKey);
	//if (g_mapLock.end() == itFind)
	{
		static CoAwaiterBool cab(1, funCancel);
		cab.m_bAwaitReady = true;
		return cab;
	}
}
