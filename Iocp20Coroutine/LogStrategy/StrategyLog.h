#pragma once
#include <glog/logging.h>
class StrategyLog
{
};

#define CHECK_NOTNULL_VOID( PTR ) \
{\
	auto* p = PTR;\
	assert(nullptr != p);\
	CHECK_NOTNULL(p);\
	if (nullptr == p) \
	{\
		return;\
	}\
}
#define CHECK_GE_VOID( val1, val2) \
{\
	CHECK_GE(val1, val2);\
	if (val1 < val2)\
		return;\
}