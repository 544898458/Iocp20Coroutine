#pragma once
#include "WorldSession.h"
class WorldServer
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompeletionKey<WorldSession>;
	void OnAdd(CompeletionKeySession&);
};

