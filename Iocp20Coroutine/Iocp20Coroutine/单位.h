#pragma once
#include "MyMsgQueue.h"
#include "BuildingComponent.h"
namespace 单位
{
	struct 单位配置
	{
		std::string strName;
		std::string strPrefabName;
	};
	struct 制造配置
	{
		uint16_t u16消耗晶体矿;
		uint16_t u16消耗燃气矿;
		uint16_t u16初始Hp;
	};
	struct 建筑单位配置
	{
		单位配置 配置;
		制造配置 建造;
		//BuildingComponent::Fun造兵 fun造兵;
		/// <summary>
		/// 静态阻挡（园的半径或正方形的半边长）
		/// </summary>
		float f半边长;
	};

	struct 活动单位配置
	{
		单位配置 配置;
		制造配置 制造;
		float f警戒距离;
	};

	struct 资源单位配置
	{
		单位配置 配置;
	};

	bool Find建筑单位配置(const 建筑单位类型 类型, 建筑单位配置& refOut);
	bool Find活动单位配置(const 活动单位类型 类型, 活动单位配置& refOut);
	bool Find资源单位配置(const 资源类型 类型, 资源单位配置& refOut);
};

