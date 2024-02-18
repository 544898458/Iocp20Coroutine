#pragma once
#include "./IocpNetwork/SessionSocketCompeletionKey.h"
#include "MsgQueue.h"
#include "Entity.h"
class MyWebSocketEndpoint;
/// <summary>
/// 对应一个网络连接
/// </summary>
class MySession
{
public:
	/// <summary>
	/// 构造函数和OnInit会紧挨着调用，初始化代码可以随便找个地方写，只要能编译通过，效果就完全相同
	/// </summary>
	MySession();
	void OnInit(Iocp::SessionSocketCompeletionKey<MySession>& refSession);
	/// <summary>
	/// 用户自定义函数，这里是纯数据，连封包概念都没有，封包是WebSocket协议负责的工作
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns></returns>
	int OnRecv(Iocp::SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
	/// <summary>
/// 从全局连接set里删除连接，从全局Space里删除实体
/// </summary>
	void OnDestroy();

	/// <summary>
	/// 发送消息给客户端
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="ref"></param>
	template<class T> void Send(const T& ref);


	/// <summary>
	/// 解析后的消息队列，解析消息在完成端口线程，处理消息在主线程（控制台界面线程）
	/// </summary>
	MsgQueue m_msgQueue;
	/// <summary>
	/// 开源WebSocket库
	/// </summary>
	std::unique_ptr<MyWebSocketEndpoint> m_webSocketEndpoint;
	/// <summary>
	/// 加入Space空间的实体（玩家角色）
	/// </summary>
	Entity m_entity;
private:
	Iocp::SessionSocketCompeletionKey<MySession> *m_pSession;
};

/// <summary>
/// 向所有连接广播消息
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="msg"></param>
template<class T>
void Broadcast(const T& msg);

/// <summary>
/// 所有活动连接
/// </summary>
extern std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_setSession;
/// <summary>
/// 多线程全局操作g_setSession
/// </summary>
extern std::mutex g_setSessionMutex;