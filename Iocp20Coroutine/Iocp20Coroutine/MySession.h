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
	/// 加入Space空间的实体（玩家角色）
	/// </summary>
	Entity m_entity;
	/// <summary>
	/// 解析后的消息队列，解析消息在完成端口线程，处理消息在主线程（控制台界面线程）
	/// </summary>
	MsgQueue m_msgQueue;
private:
	WebSocketSession<MySession>* m_pWsSession = nullptr;
};

/// <summary>
/// 向所有连接广播消息
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="msg"></param>
template<class T, class T_Session>
void Broadcast(const T& msg);