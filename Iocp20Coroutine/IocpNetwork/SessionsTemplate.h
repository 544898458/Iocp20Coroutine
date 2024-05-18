#include "Sessions.h"
#include "../LogStrategy/StrategyLog.h"

template<class T_Session>
template<typename T_Function>
void Sessions<T_Session>::AddSession(Session* pSession, T_Function const& functionLock)
{
	CHECK_PTR(pSession);
	std::lock_guard lock(m_setSessionMutex);
	m_setSession.insert(pSession);
	functionLock();
	LOG(INFO) << "Ìí¼ÓSession£¬Ê£Óà" << m_setSession.size();
}
template<class T_Session>
template<typename T_Function>
void Sessions<T_Session>::DeleteSession(Session* pSession, T_Function const& functionLock)
{
	std::lock_guard lock(m_setSessionMutex);
	m_setSession.erase(pSession);
	LOG(INFO) << "É¾³ýSession£¬Ê£Óà" << m_setSession.size();
	functionLock();
}