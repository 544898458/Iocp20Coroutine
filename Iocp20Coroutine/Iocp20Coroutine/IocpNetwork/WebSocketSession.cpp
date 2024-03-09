#include "WebSocketSession.h"


template<class T_Session>
void WebSocketSession<T_Session>::OnInit(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session> >& refSession)
{
	//std::lock_guard lock(g_setSessionMutex<WebSocketSession<T_Session> >);
	m_webSocketEndpoint.reset(new MyWebSocketEndpoint<T_Session, Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session> > >(&this->m_Session, &refSession));

	m_pSession = &refSession;
	this->m_Session.OnInit(this);
}
template<class T_Session>
int WebSocketSession< T_Session>::OnRecv(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>& refSession, const char buf[], int len)
{
	m_webSocketEndpoint->from_wire(buf, len);
	return len;
}
template<class T_Session>
void WebSocketSession< T_Session>::OnDestroy()
{
	m_Session.OnDestroy();
	

}
