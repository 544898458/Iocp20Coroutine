#pragma once
#include "MyMsgQueue.h"
#include "Entity.h"
#include "../IocpNetwork/WebSocketSession.h"

class MyServer;
class MySession 
{
public:
	MySession();
	void OnRecvWsPack(const void* buf, const int len);
	void OnInit(WebSocketSession<MySession>& refWsSession, MyServer &server);
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
	MyMsgQueue m_msgQueue;
	MyServer* m_pServer = nullptr;
private:
	WebSocketSession<MySession>* m_pWsSession = nullptr;
};
