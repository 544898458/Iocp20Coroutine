#pragma once
#include <set>
/// <summary>
/// Session集合
/// </summary>
template<class T_Session>
class Sessions
{
public:
	using Session = Iocp::SessionSocketCompletionKey<T_Session>;
	/// <summary>
	/// 向所有连接广播消息
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="msg"></param>
	template<class T>
	void Broadcast(const T& msg)
	{
		std::lock_guard lock(m_setSessionMutex);
		if (m_setSession.empty())
			return;

		//LOG(INFO) << "向" << m_setSession.size() << "个连接广播";
		for (auto p : m_setSession)
		{
			p->Session.Send(msg);
		}
	}
	template<typename T_Function> void Update(T_Function const& functionLockUpdate);
	template<typename T_Function> void AddSession(Session* pSession, T_Function const& functionLock);
	//template<typename T_Function> void DeleteSession(Session* pSession, T_Function const& functionLock);
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
	for (auto iter = m_setSession.begin(); iter != m_setSession.end(); )
	{
		Session* p = *iter;
		p->Session.m_Session.Process();

		if (!p->Finished())
		{
			++iter;
			continue;
		}

		p->Session.OnDestroy();
		delete p;
		iter = m_setSession.erase(iter);
		LOG(INFO) << "删除Session，剩余" << m_setSession.size();
	}

	functionLockUpdate();
}