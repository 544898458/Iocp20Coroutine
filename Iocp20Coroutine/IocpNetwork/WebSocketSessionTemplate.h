#include "WebSocketSession.h"

template<class T_Session>
template<class T_Server>
	requires requires(WebSocketSession<T_Session>& refWsSesson, T_Session& refSession, T_Server& server)
{
	//refSession.OnRecvWsPack((const char[])0, (const int )0	);
	refSession.OnInit(server);
}
void WebSocketSession<T_Session>::OnInit(T_Server& server)
{
	//std::lock_guard lock(g_setSessionMutex<WebSocketSession<T_Session> >);
	m_webSocketEndpoint.reset(new MyWebSocketEndpoint<T_Session, WebSocketSession<T_Session> >(&this->m_Session, this));

	this->m_Session.OnInit(server);
	m_SslTls.Init(true);
}
template<class T_Session>
int WebSocketSession< T_Session>::OnRecv(Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>& refSession, const void* buf, int len)
{
	LOG(INFO) << "收到前端发来的密文,len=" << len;
	const auto i32已处理 = m_SslTls.处理前端发来的密文(buf, len);

	获取准备发往前端的密文并发给前端();

	return i32已处理;
}
template<class T_Session>
void WebSocketSession< T_Session>::OnDestroy()
{
	m_Session.OnDestroy();


}
