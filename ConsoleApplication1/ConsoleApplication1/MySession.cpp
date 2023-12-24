#include "MySession.h"
#include "Server.cpp"
#include "ListenSocketCompeletionKey.cpp"
#include "SessionSocketCompeletionKey.cpp"

#include <msgpack.hpp>
#include <iostream>
#include <cassert>

#include <ws_endpoint.h>


template class Iocp::Server<MySession>;
template class Iocp::ListenSocketCompeletionKey<MySession>;
template class Iocp::SessionSocketCompeletionKey<MySession>;

struct MsgLogin {
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(name,pwd);
};

Iocp::SessionSocketCompeletionKey<MySession> *g_pSession;
void net_write_cb(char* buf, int64_t size, void* wd) 
{
	g_pSession->Send(buf, size);
}
WebSocketEndpoint* g_ws(nullptr);// (net_write_cb);
int MySession::OnRecv(Iocp::SessionSocketCompeletionKey<MySession>& refSession,const char buf[], int len)
{
	if (g_ws == nullptr)
	{
		g_ws = new WebSocketEndpoint(net_write_cb, &refSession);
	}
	g_pSession = &refSession;
	g_ws->from_wire(buf,len);
	//msgpack::object_handle oh =
	//	msgpack::unpack(buf, len);
	//msgpack::object obj = oh.get();
	//std::cout << obj << std::endl;
	//const auto msgLogin = obj.as<MsgLogin>() ;

	//refSession.Send(buf, len);
	return len;
}
