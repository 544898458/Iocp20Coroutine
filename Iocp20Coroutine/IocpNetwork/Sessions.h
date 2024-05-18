#pragma once
#include <set>
/// <summary>
/// Session集合
/// </summary>
template<class T_Session>
class Sessions
{
public:
	using Session = Iocp::SessionSocketCompeletionKey<T_Session>;
	/// <summary>
	/// 向所有连接广播消息
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="msg"></param>
	template<class T>
	void Broadcast(const T& msg)
	{
		std::lock_guard lock(m_setSessionMutex);
		for (auto p : m_setSession)
		{
			p->Session.Send(msg);
		}
	}
	template<typename T_Function> void Update(T_Function const& functionLockUpdate);
	template<typename T_Function> void AddSession(Session* pSession, T_Function const& functionLock);
	template<typename T_Function> void DeleteSession(Session* pSession, T_Function const& functionLock);
private:
	std::set<Session*> m_setSession;
	/// <summary>
	/// 多线程全局操作g_setSession
	/// </summary>
	std::recursive_mutex m_setSessionMutex;
};

template<class T_Session>
template<typename T_Function>
void Sessions<T_Session>::Update(T_Function const& functionLockUpdate)
{
	std::lock_guard lock(m_setSessionMutex);
	std::set<Session*> setDelete;
	for (Session* p : m_setSession)
	{
		p->Session.m_Session.m_msgQueue.Process();
		if (p->Finished())
			setDelete.insert(p);
	}
	for (auto p : setDelete)
	{
		p->Session.OnDestroy();
		delete p;
		LOG(INFO) << "已删除对象,GetCurrentThreadId=" << GetCurrentThreadId();

	}
	functionLockUpdate();
}