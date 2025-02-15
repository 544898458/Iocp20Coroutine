#include "pch.h"
#include <glog/logging.h>

#include "SocketCompeletionKey.h"

SOCKET SocketCompeletionKey::Socket() const 
{
	return socket; 
}

void SocketCompeletionKey::CloseSocket()
{
	LOG(INFO) << "CloseSocket " << socket << ",this:" << this;
	closesocket(socket);
	socket = NULL;
}
