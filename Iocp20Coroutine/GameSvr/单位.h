#pragma once
#include "MyMsgQueue.h"
//#include "BuildingComponent.h"
namespace 单位
{
	struct 单位配置
	{
		std::string strName;
		std::string strPrefabName;
		std::string str选中音效;
		std::string str空闲动作;
		std::string str阵亡动作;
		std::string str阵亡音效;
		MSGPACK_DEFINE(strName, strPrefabName, str选中音效, str空闲动作, str阵亡动作, str阵亡音效);

	};
	struct 制造配置
	{
		uint16_t u16消耗晶体矿;
		uint16_t u16消耗燃气矿;
		uint16_t u16初始Hp;
	};
	struct 建筑单位配置
	{
		/// <summary>
		/// 静态阻挡（园的半径或正方形的半边长）
		/// </summary>
		float f半边长 = 0;
	};
	struct 战斗配置
	{
		float f警戒距离;
		float f攻击距离;
		int32_t i32攻击;
		float f每帧移动距离;
		std::string str前摇动作;
		std::chrono::system_clock::duration dura开始播放攻击动作;
		std::string str攻击动作;
		std::chrono::system_clock::duration dura开始伤害;
		std::string str攻击音效;
		std::chrono::system_clock::duration dura后摇;
	};
	struct 活动单位配置
	{
		std::string str入场语音;
		std::string str走路动作;
		std::string str普通走语音;
		std::string str强行走语音;
	};
	struct 怪配置
	{
		uint16_t u16初始Hp;
	};
	
	using 单位属性等级配置 = std::map<单位属性类型, std::map<uint16_t, uint16_t> >;
	//struct 资源单位配置
	//{
	//	单位配置 配置;
	//};

	bool 读配置文件();
	bool Find建筑单位配置(const 单位类型 类型, 建筑单位配置& refOut);
	bool Find活动单位配置(const 单位类型 类型, 活动单位配置& refOut);
	bool Find单位配置(const 单位类型 类型, 单位配置& refOut);
	bool Find战斗配置(const 单位类型 类型, 战斗配置& refOut);
	bool Find怪配置(const 单位类型 类型, 怪配置& refOut);
	bool Find制造配置(const 单位类型 类型, 制造配置& refOut);
	bool Find单位属性等级配置(const 单位类型 单位, const 单位属性类型 属性, const uint16_t u16等级, uint16_t& refOut加数值);
};

