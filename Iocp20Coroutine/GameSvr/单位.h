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
		float f半边长 = 0;
	};
	struct 战斗配置
	{
		float f警戒距离;
		float f攻击距离;
		float f伤害;
		float f每帧移动距离;
		std::string str前摇动作;
		std::chrono::system_clock::duration dura开始播放攻击动作;
		std::string str攻击动作;
		std::chrono::system_clock::duration dura开始伤害;
		std::string str攻击音效;
		
	};
	struct 活动单位配置
	{
		单位配置 配置;
		制造配置 制造;
		战斗配置 战斗;
	};
	struct 怪配置
	{
		单位配置 配置;
		战斗配置 战斗;
		uint16_t u16初始Hp;
	};
	
	struct 资源单位配置
	{
		单位配置 配置;
	};

	bool Find建筑单位配置(const 建筑单位类型 类型, 建筑单位配置& refOut);
	bool Find活动单位配置(const 活动单位类型 类型, 活动单位配置& refOut);
	bool Find资源单位配置(const 资源类型 类型, 资源单位配置& refOut);
	bool Find怪配置(const 活动单位类型 类型, 怪配置& refOut);
};

