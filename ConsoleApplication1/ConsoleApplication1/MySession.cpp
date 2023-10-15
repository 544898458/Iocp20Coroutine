#include "MySession.h"
#include "Server.cpp"
#include "ListenSocketCompeletionKey.cpp"
#include "SessionSocketCompeletionKey.cpp"
template class Iocp::Server<MySession>;
template class ListenSocketCompeletionKey<MySession>;
template class SessionSocketCompeletionKey<MySession>;

int MySession::OnRecv(const char buf[], int len)
{
	return len;
}
