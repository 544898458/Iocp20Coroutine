#pragma once
#include <set>
/// <summary>
/// Session����
/// </summary>
template<class T_Session>
class Sessions
{
public:
	using Session = Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session> >;
	/// <summary>
	/// ���������ӹ㲥��Ϣ
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="msg"></param>
	template<class T>
	void Broadcast(const T& msg)
	{
		std::lock_guard lock(m_setSessionMutex);
		for (auto p : m_setSession)
		{
			p->Session.m_Session.Send(msg);
		}
	}
	template<typename T_Function> void Update(T_Function const& functionLockUpdate);
	template<typename T_Function> void AddSession(Session* pSession, T_Function const& functionLock);
	template<typename T_Function> void DeleteSession(Session* pSession, T_Function const& functionLock);
private:
	std::set<Session*> m_setSession;
	/// <summary>
	/// ���߳�ȫ�ֲ���g_setSession
	/// </summary>
	std::recursive_mutex m_setSessionMutex;
};