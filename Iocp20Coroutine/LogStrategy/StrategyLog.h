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
#define CHECK_NOTNULL_CO_RET_0( PTR ) \
{\
	if (nullptr == (PTR)) \
	{\
		LOG(ERROR)<< #PTR << ",is null";\
		co_return 0;\
	}\
}

#define CHECK_CO_RET_0( EXP ) \
{\
	if (!(EXP)) \
	{\
		LOG(ERROR)<< #EXP<< "is false";\
		co_return 0;\
	}\
}
#define CHECK_RET_FALSE( EXP ) \
{\
	if (!(EXP)) \
	{\
		LOG(ERROR)<< #EXP ;\
		return false;\
	}\
}

#define CHECK_RET_VOID( EXP ) \
{\
	if (!(EXP)) \
	{\
		LOG(ERROR)<< #EXP ;\
		return ;\
	}\
}
#define CHECK_NOTNULL_RET_FALSE( PTR ) \
{\
	if (nullptr == (PTR)) \
	{\
		LOG(ERROR)<< #PTR << ",is null";\
		return false;\
	}\
}

#define CHECK_GE_VOID( val1, val2) \
{\
	CHECK_GE(val1, val2);\
	if (val1 < val2)\
		return;\
}

#define CHECK_VOID( exp ) \
{\
	CHECK(exp);\
	if (!exp)\
		return;\
}

#define CHECK_CO_RET_FALSE( exp ) \
{\
	CHECK(exp);\
	if (!exp)\
		co_return false;\
}

#define CHECK_DEFAULT( exp ) \
{\
	CHECK(exp);\
	if (!exp)\
		return {};\
}

#define CHECK_FALSE( exp ) \
{\
	CHECK(exp);\
	if (!exp)\
		return false;\
}