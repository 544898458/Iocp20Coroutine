#include "pch.h"
#include <glog/logging.h>

#include "SocketCompeletionKey.h"

SOCKET SocketCompeletionKey::Socket() const 
{
	return socket; 
}

void SocketCompeletionKey::CloseSocket()
{
	if (INVALID_SOCKET == socket)
		return;

	LOG(INFO) << "CloseSocket " << socket << ",this:" << this;
	closesocket(socket);
	socket = INVALID_SOCKET;
}
