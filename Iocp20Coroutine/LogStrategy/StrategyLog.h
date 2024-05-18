#pragma once
class StrategyLog
{
};

#define CHECK_PTR( PTR ) \
{\
	auto* p = PTR;\
	assert(nullptr != p);\
	if (nullptr == p) \
	{\
		LOG(INFO) << #PTR "ÊÇnullptr";\
		return;\
	}\
}