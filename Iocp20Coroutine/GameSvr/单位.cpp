#include "pch.h"
#include "单位.h"
#include <unordered_map>
namespace 单位
{

	std::unordered_map<建筑单位类型, 建筑单位配置> g_map建筑单位配置 =
	{
		//								晶体燃气HP
		{基地, {{"基地","house_type06"},{20, 0,200}, 2.0f}},
		{兵厂, {{"兵厂","house_type19"},{15,11,100},2.0f} },
		{民房, {{"民房","house_type21"},{10, 8,50}, 1.0f} },
		{地堡, {{"地堡","house_type17"},{15,15,500}, 2.0f} },
	};

	std::unordered_map<活动单位类型, 活动单位配置> g_map活动单位配置 =
	{
		//										晶体燃气HP	警戒	攻击	
		{工程车,{{"工程车"	, "altman-yellow"	},{ 3,0,20	}, 8 , 4} },
		{兵,	{{"兵"		, "altman-blue"		},{ 5,6,15	}, 20, 9} },
		{近战兵,{{"近战兵"	, "axe-yellow"		},{ 8,8,18	}, 15, 4} },
	};
	std::unordered_map<资源类型, 资源单位配置> g_map资源单位配置 =
	{
		{晶体矿,{{"晶体矿"	, "tree_large"	}}},
		{燃气矿,{{"燃气矿"	, "house_type03"}}},
	};

	template<class K, class V>
	bool Find配置(std::unordered_map<K, V>& refMap, const K 类型, V& refOut)
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
	bool Find资源单位配置(const 资源类型 类型, 资源单位配置& refOut)
	{
		return Find配置(g_map资源单位配置, 类型, refOut);
	}
}