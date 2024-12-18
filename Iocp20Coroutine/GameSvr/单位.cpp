#include "pch.h"
#include "单位.h"
#include <unordered_map>
namespace 单位
{

	std::unordered_map<建筑单位类型, 建筑单位配置> g_map建筑单位配置 =
	{
		//								晶体燃气HP
		{基地, {{"基地","建筑/基地"},{50, 0,200}, 5.0f}},
		{兵厂, {{"兵厂","house_type19"},{25, 9,100},2.0f} },
		{民房, {{"民房","house_type21"},{20, 3,50}, 1.0f} },
		{地堡, {{"地堡","house_type17"},{30,11,500}, 2.0f} },
	};

	std::unordered_map<活动单位类型, 活动单位配置> g_map活动单位配置 =
	{
		//										晶体燃气HP	  警戒 攻击	
		{工程车,{{"工程车"	, "altman-yellow"	},{ 3,0,20	}, {8 , 4, 1}} },
		{兵,	{{"兵"		, "活动单位/步兵"	},{ 5,2,15	}, {20, 9, 3}} },
		{近战兵,{{"近战兵"	, "axe-yellow"		},{ 8,3,18	}, {15, 4, 5}} },
	};
	std::unordered_map<活动单位类型, 怪配置> g_map怪配置 = {
		//									警距 攻距 伤	血量
		{兵,	{{"怪"		, "altman-red"	},{20,8,5}, 50} },
		{近战兵,{{"斧头怪"	, "axe-red"		},{15,3,8}, 65} },
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
	bool Find怪配置(const 活动单位类型 类型, 怪配置& refOut)
	{
		return Find配置(g_map怪配置, 类型, refOut);
	}
}