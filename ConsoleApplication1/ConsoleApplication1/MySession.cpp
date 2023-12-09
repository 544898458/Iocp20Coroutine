#include "MySession.h"
#include "Server.cpp"
#include "ListenSocketCompeletionKey.cpp"
#include "SessionSocketCompeletionKey.cpp"

#include <msgpack.hpp>
#include <iostream>
#include <cassert>

template class Iocp::Server<MySession>;
template class Iocp::ListenSocketCompeletionKey<MySession>;
template class Iocp::SessionSocketCompeletionKey<MySession>;

struct MsgLogin {
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(name,pwd);
};

int MySession::OnRecv(Iocp::SessionSocketCompeletionKey<MySession>& refSession,const char buf[], int len)
{
	msgpack::object_handle oh =
		msgpack::unpack(buf, len);
	msgpack::object obj = oh.get();
	std::cout << obj << std::endl;
	const auto msgLogin = obj.as<MsgLogin>() ;

	refSession.Send(buf, len);
	return len;
}
