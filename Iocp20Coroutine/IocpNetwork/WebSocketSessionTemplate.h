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
	m_webSocketEndpoint.reset(new MyWebSocketEndpoint<T_Session, Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session> > >(&this->m_Session, &m_refSession));

	this->m_Session.OnInit(server);
	m_SslTls.Init(true);
}
template<class T_Session>
int WebSocketSession< T_Session>::OnRecv(Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>& refSession, const void* buf, int len)
{
	const auto i32已处理 = m_SslTls.处理前端发来的密文(buf, len);

	{
		char buf密文[4096];
		int len密文 = m_SslTls.获取准备发往前端的密文(buf密文);
		if (0 < len密文)
			Send密文(buf密文, len密文);
	}

	{
		char buf明文[4096];
		int len明文 = m_SslTls.读出已解密的明文(buf明文);
		if (0 < len明文)
			m_webSocketEndpoint->from_wire(buf明文, len明文);
	}

	return i32已处理;
}
template<class T_Session>
void WebSocketSession< T_Session>::OnDestroy()
{
	m_Session.OnDestroy();


}
