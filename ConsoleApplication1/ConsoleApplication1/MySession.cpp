#include "MySession.h"
#include "Server.cpp"
#include "ListenSocketCompeletionKey.cpp"
#include "SessionSocketCompeletionKey.cpp"
template class Iocp::Server<MySession>;
template class Iocp::ListenSocketCompeletionKey<MySession>;
template class Iocp::SessionSocketCompeletionKey<MySession>;

int MySession::OnRecv(Iocp::SessionSocketCompeletionKey<MySession>& refSession,const char buf[], int len)
{
	refSession.Send(buf, len);
	return len;
}
