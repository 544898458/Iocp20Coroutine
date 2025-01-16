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
		assert(false);\
		return;\
	}\
}
#define CHECK_NOTNULL_CO_RET_0( PTR ) \
{\
	if (nullptr == (PTR)) \
	{\
		LOG(ERROR)<< #PTR << ",is null";\
		assert(false);\
		co_return 0;\
	}\
}
#define CHECK_NOTNULL_CO_RET_FALSE( PTR ) \
{\
	if (nullptr == (PTR)) \
	{\
		LOG(ERROR)<< #PTR << ",is null";\
		assert(false);\
		co_return false;\
	}\
}
#define CHECK_CO_RET_0( EXP ) \
{\
	if (!(EXP)) \
	{\
		LOG(ERROR)<< #EXP<< "is false";\
		assert(false);\
		co_return 0;\
	}\
}
#define CHECK_RET_FALSE( EXP ) \
{\
	if (!(EXP)) \
	{\
		LOG(ERROR)<< #EXP ;\
		assert(false);\
		return false;\
	}\
}

#define CHECK_RET_VOID( EXP ) \
{\
	if (!(EXP)) \
	{\
		LOG(ERROR)<< #EXP ;\
		assert(false);\
		return ;\
	}\
}
#define CHECK_RET_DEFAULT( EXP ) \
{\
	if (!(EXP)) \
	{\
		LOG(ERROR)<< #EXP ;\
		assert(false);\
		return {};\
	}\
}
#define CHECK_NOTNULL_RET_FALSE( PTR ) \
{\
	if (nullptr == (PTR)) \
	{\
		LOG(ERROR)<< #PTR << ",is null";\
		assert(false);\
		return false;\
	}\
}

#define CHECK_GE_VOID( val1, val2) \
{\
	if (val1 < val2)\
	{\
		LOG(ERROR) << #val1 << ":" << val1 << ">=" << #val2 << ":" << val2 ;\
		assert(false);\
		return;\
	}\
}

#define CHECK_VOID( exp ) \
{\
	LOG_IF(ERROR, !(exp));\
	if (!(exp))\
	{\
		assert(false);\
		return; \
	}\
}

#define CHECK_CO_RET_FALSE( exp ) \
{\
	LOG_IF(ERROR, !(exp));\
	if (!(exp))\
	{\
		assert(false);\
		co_return false; \
	}\
}

#define CHECK_DEFAULT( exp ) \
{\
	LOG_IF(ERROR, !(exp));\
	if (!(exp))\
	{\
		return {}; \
	}\
}

#define CHECK_FALSE( exp ) \
{\
	LOG_IF(ERROR, !(exp));\
	if (!(exp))\
	{\
		assert(false);\
		return false; \
	}\
}

#define CHECK_WP_CO_RET_FALSE( wp ) \
{\
	LOG_IF(ERROR, wp.expired());\
	if ((wp).expired())\
	{\
		assert(false);\
		co_return false; \
	}\
}
#define CHECK_WP_CO_RET_0( wp ) \
{\
	LOG_IF(ERROR, wp.expired());\
	if ((wp).expired())\
	{\
		assert(false);\
		co_return 0; \
	}\
}
#define CHECK_WP_CONTINUE( wp ) \
{\
	if ((wp).expired()) \
	{\
		LOG(ERROR)<< #wp<< ",expired";\
		assert(false);\
		continue;\
	}\
}

#define CHECK_CONTINUE( exp ) \
{\
	if (!(exp)) \
	{\
		LOG(ERROR)<< #exp << ",false";\
		assert(false);\
		continue;\
	}\
}

#define CHECK_WP_RET_VOID( wp ) \
{\
	if ((wp).expired()) \
	{\
		LOG(ERROR)<< #wp<< ",expired";\
		assert(false);\
		return;\
	}\
}

#define CHECK_WP_RET_DEFAULT( wp ) \
{\
	if ((wp).expired()) \
	{\
		LOG(ERROR)<< #wp<< ",expired";\
		assert(false);\
		return {};\
	}\
}

#define CHECK_WP_RET_FALSE( wp ) \
{\
	if ((wp).expired()) \
	{\
		LOG(ERROR)<< #wp<< ",expired";\
		assert(false);\
		return false;\
	}\
}

#define CHECK_AOI_CONTINUE( ref ) \
{\
	if (!(ref).m_upAoi) \
	{\
		LOG(ERROR)<< #ref << ",m_upAoi";\
		assert(false);\
		continue;\
	}\
}