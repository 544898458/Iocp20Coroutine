#pragma once
#include <glog/logging.h>
class StrategyLog
{
};

#define CHECK_NOTNULL_VOID( PTR ) \
{\
	if (nullptr == (PTR)) \
	{\
		LOG(ERROR)<< #PTR << ",is null";\
		return;\
	}\
}
#define CHECK_GE_VOID( val1, val2) \
{\
	CHECK_GE(val1, val2);\
	if (val1 < val2)\
		return;\
}