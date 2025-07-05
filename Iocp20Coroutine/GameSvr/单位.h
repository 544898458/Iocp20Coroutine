#pragma once
#include "MyMsgQueue.h"

class Space;
namespace 单位
{
	struct 动作
	{
		std::string str名字或索引;
		float f播放速度 = 1.0f;
		float f起始时刻秒 = 0;
		float f结束时刻秒 = 0;
		MSGPACK_DEFINE(str名字或索引, f播放速度, f起始时刻秒, f结束时刻秒);
	};
	
	/// <summary>
	/// 序列化进混战地图里，只能在后面加字段，不能插入字段
	/// </summary>
	struct 单位配置
	{
		std::string strName;
		种族 种族;
		std::string strPrefabName;
		bool b骨骼动画 = true;
		std::string str选中音效;
		动作 空闲;
		动作 阵亡;
		std::string str阵亡音效;
		MSGPACK_DEFINE(strName, 种族, strPrefabName, b骨骼动画	, str选中音效, 空闲, 阵亡, str阵亡音效);
	};
	struct 消耗资源
	{
		uint16_t u16消耗晶体矿;
		uint16_t u16消耗燃气矿;
	};
	struct 制造配置
	{
		消耗资源 消耗;
		uint16_t u16初始Hp;
		单位类型 前置单位;
		uint16_t u16耗时帧;
	};
	struct 建筑单位配置
	{
		/// <summary>
		/// 静态阻挡（园的半径或正方形的半边长）
		/// </summary>
		float f半边长 = 0;
		动作 建造;
	};
	struct 战斗配置
	{
		float f警戒距离;
		float f攻击距离;
		uint16_t u16攻击;
		uint16_t u16防御;
		float f每帧移动距离;
		std::string str前摇动作;
		std::chrono::system_clock::duration dura开始播放攻击动作;
		std::string str攻击动作;
		std::string str弹丸特效;
		uint16_t u16开始伤害;
		std::string str攻击音效;
		std::chrono::system_clock::duration dura后摇;
		bool b空中 = false;
		bool b可打空中 = false;
	};
	struct 活动单位配置
	{
		std::string str入场语音;
		动作 走路;
		std::string str普通走语音;
		std::string str强行走语音;
	};
	struct 怪配置
	{
		uint16_t u16初始Hp;
	};
	struct 单位属性等级配置详情
	{
        float f数值;
		消耗资源 消耗;
	};
	using 单位属性等级配置 = std::map<属性类型, std::map<uint16_t, 单位属性等级配置详情> >;

	struct Buff配置
	{
		属性类型 属性;
		float f变化值;
		std::chrono::system_clock::duration dura间隔;
	};

	struct 战局配置
	{
		typedef CoTask<int>(*funCo副本剧情)(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);

		战局类型 id副本;
		std::string strSceneName;
		std::string str寻路文件名;
		std::string strHttps音乐;
		bool Is多人战局()const;
	};


	bool 读配置文件();
	bool Find建筑单位配置(const 单位类型 类型, 建筑单位配置& refOut);
	bool Find活动单位配置(const 单位类型 类型, 活动单位配置& refOut);
	bool Find单位配置(const 单位类型 类型, 单位配置& refOut);
	bool Find战斗配置(const 单位类型 类型, 战斗配置& refOut);
	bool Find怪配置(const 单位类型 类型, 怪配置& refOut);
	bool Find制造配置(const 单位类型 类型, 制造配置& refOut);
	bool Find单位解锁配置(const 单位类型 单位, 消耗资源& refOut);
	bool Find单位属性等级配置(const 单位类型 单位, const 属性类型 属性, const uint16_t u16等级, 单位属性等级配置详情& refOut);
	bool FindBuff配置(const BuffId 类型, Buff配置& refOut);
	bool Find战局配置(const 战局类型 类型, 战局配置& refOut);

	template<typename T_成员>
	T_成员 单位升级后属性(const 单位类型 单位, const 属性类型 属性, const uint16_t u16等级, T_成员 战斗配置::* p成员);
	bool Is虫(const 单位类型 单位);
	//uint16_t 单位攻击(const 单位类型 单位, const uint16_t u16攻击等级);
	//uint16_t 单位防御(const 单位类型 单位, const uint16_t u16防御等级);
	//float 单位速度每帧移动距离(const 单位类型 单位, const uint16_t u16移速等级);
};