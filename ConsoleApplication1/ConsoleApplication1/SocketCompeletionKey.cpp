#include "SocketCompeletionKey.h"

SOCKET SocketCompeletionKey::Socket() const 
{
	return socket; 
}

void SocketCompeletionKey::CloseSocket()
{
	printf("CloseSocket %d\n", socket);
	closesocket(socket);
	socket = NULL;
}
