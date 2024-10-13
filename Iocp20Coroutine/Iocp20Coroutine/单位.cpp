#include "pch.h"
#include "单位.h"
#include <unordered_map>
namespace 单位
{
	std::unordered_map<建筑单位类型, 建筑单位配置> g_map建筑单位配置 =
	{
		{基地, {{"基地","house_type06",1},&BuildingComponent::Co造工程车}},
		{兵厂, {{"兵厂","house_type19",2},&BuildingComponent::Co造兵}},
	};

	std::unordered_map<活动单位类型, 活动单位配置> g_map活动单位配置 =
	{
		{工程车,{{"工程车","altman-yellow",3}}},
		{兵,	{{"兵","altman-blue",5}}},
	};

	template<class K,class V>
	bool Find配置(std::unordered_map<K,V> &refMap, const K 类型, V& refOut)
	{
		const auto& iterFind = refMap.find(类型);
		if (iterFind == refMap.end())
		{
			assert(false);
			return false;
		}

		refOut = iterFind->second;
		return true;
	}
	bool Find建筑单位配置(const 建筑单位类型 类型, 建筑单位配置& refOut)
	{
		return Find配置(g_map建筑单位配置, 类型, refOut);
	}

	bool Find活动单位配置(const 活动单位类型 类型, 活动单位配置& refOut)
	{
		return Find配置(g_map活动单位配置, 类型, refOut);
	}
}