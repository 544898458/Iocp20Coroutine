#include "Sessions.h"
#include "../LogStrategy/StrategyLog.h"

/// <summary>
/// 网络线程，多线程
/// </summary>
/// <typeparam name="T_Session"></typeparam>
/// <param name="pSession"></param>
/// <param name="functionLock"></param>
/// <param name="idSession"></param>
template<class T_Session>
template<typename T_Function>
void Sessions<T_Session>::AddSession(Session* pSession, T_Function const& functionLock, const uint64_t idSession)
{
	CHECK_NOTNULL_VOID(pSession);
	std::lock_guard lock(m_setSessionMutex);
	auto pair = m_mapSession.insert({ idSession, pSession });
	
	functionLock();
	LOG(INFO) << "添加Session，添加后数量:" << m_mapSession.size();
}
//template<class T_Session>
//template<typename T_Function>
//void Sessions<T_Session>::DeleteSession(Session* pSession, T_Function const& functionLock)
//{
//	std::lock_guard lock(m_setSessionMutex);
//	m_setSession.erase(pSession);
//	LOG(INFO) << "删除Session，剩余" << m_setSession.size();
//	functionLock();
//}