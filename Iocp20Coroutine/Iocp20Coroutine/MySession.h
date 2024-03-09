#pragma once
#include "MsgQueue.h"
#include "Entity.h"
#include "./IocpNetwork/WebSocketSession.h"

class MySession 
{
public:
	MySession();
	void OnRecvWsPack(const char buf[], const int len);
	void OnInit(WebSocketSession<MySession>* pWsSession);
	void OnDestroy();
	template<class T>
	void Send(const T& ref);
	/// <summary>
	/// ����Space�ռ��ʵ�壨��ҽ�ɫ��
	/// </summary>
	Entity m_entity;
	/// <summary>
	/// ���������Ϣ���У�������Ϣ����ɶ˿��̣߳�������Ϣ�����̣߳�����̨�����̣߳�
	/// </summary>
	MsgQueue m_msgQueue;
private:
	WebSocketSession<MySession>* m_pWsSession = nullptr;
};


/// <summary>
/// ���л����
/// </summary>
template<class T_Session>
std::set<Iocp::SessionSocketCompeletionKey<T_Session>*> g_setSession;
/// <summary>
/// ���߳�ȫ�ֲ���g_setSession
/// </summary>
template<class T_Session>
std::mutex g_setSessionMutex;


/// <summary>
/// ���������ӹ㲥��Ϣ
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="msg"></param>
template<class T, class T_Session>
void Broadcast(const T& msg);