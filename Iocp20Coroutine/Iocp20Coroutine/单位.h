#pragma once
#include "MyMsgQueue.h"
#include "BuildingComponent.h"
namespace 单位
{
	struct 单位配置
	{
		std::string strName;
		std::string strPrefabName;
		uint32_t u32消耗钱;
	};

	struct 建筑单位配置
	{
		单位配置 配置;
		BuildingComponent::Fun造兵 fun造兵;
	};

	struct 活动单位配置
	{
		单位配置 配置;
	};

	bool Find建筑单位配置(const 建筑单位类型 类型, 建筑单位配置& refOut);
	bool Find活动单位配置(const 活动单位类型 类型, 活动单位配置& refOut);
};

