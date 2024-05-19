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
	/// 加入Space空间的实体（玩家角色）
	/// </summary>
	Entity m_entity;
	/// <summary>
	/// 解析后的消息队列，解析消息在完成端口线程，处理消息在主线程（控制台界面线程）
	/// </summary>
	MyMsgQueue m_msgQueue;
	MyServer* m_pServer = nullptr;
private:
	WebSocketSession<MySession>* m_pWsSession = nullptr;
};
