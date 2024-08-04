#include "WebSocketSession.h"

template<class T_Session>
template<class T_Server>
void WebSocketSession<T_Session>::OnInit(Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session> >& refSession, T_Server&server)
{
	//std::lock_guard lock(g_setSessionMutex<WebSocketSession<T_Session> >);
	m_webSocketEndpoint.reset(new MyWebSocketEndpoint<T_Session, Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session> > >(&this->m_Session, &refSession));

	m_pSession = &refSession;
	this->m_Session.OnInit(*this, server);
}
template<class T_Session>
int WebSocketSession< T_Session>::OnRecv(Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>& refSession, const void* buf, int len)
{
	m_webSocketEndpoint->from_wire(buf, len);
	return len;
}
template<class T_Session>
void WebSocketSession< T_Session>::OnDestroy()
{
	m_Session.OnDestroy();
	

}
