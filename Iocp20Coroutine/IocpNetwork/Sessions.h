#pragma once
#include <map>
/// <summary>
/// Session����
/// </summary>
template<class T_Session>
class Sessions
{
public:
	using Session = Iocp::SessionSocketCompletionKey<T_Session>;

	/// <summary>
	/// ���������ӹ㲥��Ϣ
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="msg"></param>
	template<class T>
	void Broadcast(const T& msg)
	{
		//std::lock_guard lock(m_setSessionMutex);
		if (m_mapSession.empty())
			return;

		//LOG(INFO) << "��" << m_setSession.size() << "�����ӹ㲥";
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
			//_ASSERTfalse);
			LOG(WARNING) << "Sessions�Ҳ���idSession=" << idSession;
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
	/// ���߳�ȫ�ֲ���g_setSession
	/// </summary>
	std::recursive_mutex m_setSessionMutex;
};

/// <summary>
/// ���̣߳������߳�
/// </summary>
/// <typeparam name="T_Session"></typeparam>
/// <param name="functionLockUpdate"></param>
template<class T_Session>
template<typename T_Function>
void Sessions<T_Session>::Update(T_Function const& functionLockUpdate)
{
	std::lock_guard lock(m_setSessionMutex);
	for (auto iter = m_mapSession.begin(); iter != m_mapSession.end(); )
	{
		Session* p = iter->second;
		if (!p->Session.Process())
			p->Session.m_refSession.CloseSocket();

		if (!p->Finished())
		{
			++iter;
			continue;
		}

		p->Session.OnDestroy();
		iter = m_mapSession.erase(iter);
		LOG(INFO) << "ɾ��Session"<< p <<"��ʣ��" << m_mapSession.size();
		delete p;
	}

	functionLockUpdate();
}