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
