#pragma once
#include <map>
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
		if (m_mapSession.empty())
			return;

		//LOG(INFO) << "向" << m_setSession.size() << "个连接广播";
		for (auto& [k,v] : m_mapSession)
		{
			v->Session.Send(msg);
		}
	}
	Session* GetSession(const uint64_t idSession)
	{
		auto itFind = m_mapSession.find(idSession);
		if (itFind == m_mapSession.end())
		{
			//assert(false);
			LOG(WARNING) << "ERR";
			return nullptr;
		}
		return itFind->second;
	}
	template<typename T_Function> void Update(T_Function const& functionLockUpdate);
	template<typename T_Function> void AddSession(Session* pSession, T_Function const& functionLock, const uint64_t idSession);
	//template<typename T_Function> void DeleteSession(Session* pSession, T_Function const& functionLock);
private:
	std::map<uint64_t, Session*> m_mapSession;
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
	for (auto iter = m_mapSession.begin(); iter != m_mapSession.end(); )
	{
		Session* p = iter->second;
		p->Session.Process();

		if (!p->Finished())
		{
			++iter;
			continue;
		}

		p->Session.OnDestroy();
		delete p;
		iter = m_mapSession.erase(iter);
		LOG(INFO) << "删除Session，剩余" << m_mapSession.size();
	}

	functionLockUpdate();
}