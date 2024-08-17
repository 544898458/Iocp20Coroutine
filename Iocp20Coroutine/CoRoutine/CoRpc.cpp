#include "CoRpc.h"

namespace CoRpc
{
	int g_rpcSnId = 0;
	std::map<int, CoAwaiter> g_mapRpc;
	std::map<int, FunCancel> g_mapRpcCancel;
}