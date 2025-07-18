#include "pch.h"
#include "单位.h"
#include "枚举/BuffId.h"
#include "枚举/属性类型.h"
#include "枚举/单位类型.h"
#include <unordered_map>

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include <fstream>
#include "../IocpNetwork/StrConv.h"
#ifdef _DEBUG
#pragma comment(lib, "../yaml-cpp-0.8.0/lib/yaml-cppd.lib")
#else
#pragma comment(lib, "../yaml-cpp-0.8.0/lib/yaml-cpp.lib")
#endif

namespace std
{
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::动作& _ref)
	{
		return _Ostr << _ref.str名字或索引 << "," << _ref.f播放速度 << "," << _ref.f起始时刻秒 << "," << _ref.f结束时刻秒;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::单位配置& _ref)
	{
		return _Ostr << "单位配置:" << _ref.strName << "\t" << _ref.种族 << "\t" << _ref.strPrefabName << "\t" << _ref.b骨骼动画 << "\t" << _ref.str选中音效 << "\t" << _ref.空闲 << "\t" << _ref.阵亡 << "\t" << _ref.str阵亡音效;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::战局配置& _ref)
	{
		return _Ostr << "战局配置:" << _ref.id副本 << "\t" << _ref.strSceneName << "\t" << _ref.str寻路文件名 << "\t" << _ref.strHttps音乐;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::战斗配置& _ref)
	{
		return _Ostr << "战斗配置," << _ref.f警戒距离 << "\t" << _ref.f攻击距离 << "\t" << _ref.u16攻击 << "\t" << _ref.u16防御 << "\t" << _ref.f每帧移动距离
			<< _ref.str前摇动作 << "\t" << _ref.dura开始播放攻击动作 << "\t" << _ref.攻击 << "\t" << _ref.u16开始伤害
			<< _ref.str攻击音效 << "\t" << _ref.dura后摇 << "\t" << _ref.b空中 << "\t" << _ref.b可打空中 << "\t" << _ref.b可打地面;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::活动单位配置& _ref)
	{
		return _Ostr << "活动单位配置," << _ref.str入场语音 << "," << _ref.走路 << "," << _ref.str普通走语音 << "," << _ref.str强行走语音;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::建筑单位配置& _ref)
	{
		return _Ostr << "建筑单位配置," << _ref.f半边长 << "," << _ref.建造;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::消耗资源& _ref)
	{
		return _Ostr << "消耗资源," << _ref.u16消耗晶体矿 << "\t" << _ref.u16消耗燃气矿;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::制造配置& _ref)
	{
		return _Ostr << "制造配置," << _ref.消耗 << "\t" << _ref.u16初始Hp << "\t" << _ref.u16耗时帧 << "\t" << _ref.前置单位;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::怪配置& _ref)
	{
		return _Ostr << "怪配置," << _ref.u16初始Hp;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::单位属性等级配置& _ref)
	{
		return _Ostr << "单位属性等级配置,size=" << _ref.size();
	}

	template <class _Traits> std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::Buff配置& _ref)
	{
		return _Ostr << "Buff配置," << _ref.属性 << "," << _ref.f变化值 << "," << _ref.dura间隔;
	}
}

namespace YAML {
	// 辅助函数：安全获取YAML节点值，支持默认值
	template<typename T>
	T safe_get(const Node& node, const std::string& key, const T& default_value) {
		return node[key] ? node[key].as<T>() : default_value;
	}

	// 辅助函数：安全获取时间值
	std::chrono::milliseconds safe_get_duration(const Node& node, const std::string& key, int32_t default_ms = 0) {
		return node[key] ? std::chrono::milliseconds(node[key].as<int32_t>()) : std::chrono::milliseconds(default_ms);
	}

	// 辅助函数：安全获取时间值（int16_t版本）
	std::chrono::milliseconds safe_get_duration16(const Node& node, const std::string& key, int16_t default_ms = 0) {
		return node[key] ? std::chrono::milliseconds(node[key].as<int16_t>()) : std::chrono::milliseconds(default_ms);
	}

	template<>
	struct convert<战局类型> {
		static Node encode(const 战局类型& rhs) {

			return Node((int)rhs);
		}
		static bool decode(const Node& refNode, 战局类型& rhs) {
			CHECK_RET_FALSE(refNode.IsScalar());
			rhs = (战局类型)refNode.as<int>();
			return true;
		}
	};
	template<>
	struct convert<单位类型> {
		static Node encode(const 单位类型& rhs) {

			return Node((int)rhs);
		}
		static bool decode(const Node& refNode, 单位类型& rhs) {
			CHECK_RET_FALSE(refNode.IsScalar());
			rhs = (单位类型)refNode.as<int>();
			return true;
		}
	};
	template<>
	struct convert<属性类型> {
		static Node encode(const 属性类型& rhs) {

			return Node((int)rhs);
		}
		static bool decode(const Node& refNode, 属性类型& rhs) {
			CHECK_RET_FALSE(refNode.IsScalar());
			rhs = (属性类型)refNode.as<int>();
			return true;
		}
	};
	template<>
	struct convert<BuffId> {
		static Node encode(const BuffId& rhs) {

			return Node((int)rhs);
		}
		static bool decode(const Node& refNode, BuffId& rhs) {
			CHECK_RET_FALSE(refNode.IsScalar());
			rhs = (BuffId)refNode.as<int>();
			return true;
		}
	};
	template<>
	struct convert<种族> {
		static Node encode(const 种族& rhs) {

			return Node((int)rhs);
		}
		static bool decode(const Node& refNode, 种族& rhs) {
			CHECK_RET_FALSE(refNode.IsScalar());
			rhs = (种族)refNode.as<int>();
			return true;
		}
	};
	template<>
	struct convert<单位::动作> {
		static Node encode(const 单位::动作& rhs) {
			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::动作& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { 
				safe_get<std::string>(refNode, "名字或索引", ""),
				safe_get<float>(refNode, "播放速度", 1.0f),
				safe_get<float>(refNode, "起始时刻秒", 0.0f),
				safe_get<float>(refNode, "结束时刻秒", 0.0f)
			};
			return true;
		}
	};
	static const 单位::动作 默认动作 = {"", 1.0f, 0.0f, 0.0f};
	template<>
	struct convert<单位::单位配置> {
		static Node encode(const 单位::单位配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::单位配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			
			rhs = { 
				safe_get<std::string>(refNode, "名字", ""),
				safe_get<种族>(refNode, "种族", 种族(0)),
				safe_get<std::string>(refNode, "PrefabPathName", ""),
				safe_get<bool>(refNode, "是骨骼动画", false),
				safe_get<std::string>(refNode, "选中音效", ""),
				safe_get<单位::动作>(refNode, "空闲动作", 默认动作),
				safe_get<单位::动作>(refNode, "阵亡动作", 默认动作),
				safe_get<std::string>(refNode, "阵亡音效", ""),
			};
			return true;
		}
	};
	template<>
	struct convert<单位::战斗配置> {
		static Node encode(const 单位::战斗配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::战斗配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { 
				safe_get<float>(refNode, "f警戒距离", 0.0f),
				safe_get<float>(refNode, "f攻击距离", 0.0f),
				safe_get<uint16_t>(refNode, "攻击", 0),
				safe_get<uint16_t>(refNode, "防御", 0),
				safe_get<float>(refNode, "f每帧移动距离", 0.0f),
				safe_get<std::string>(refNode, "str前摇动作", ""),
				safe_get_duration(refNode, "dura开始播放攻击动作", 0),
				safe_get<单位::动作>(refNode, "攻击动作", 默认动作),
				safe_get<std::string>(refNode, "str弹丸特效", ""),
				safe_get<uint16_t>(refNode, "dura开始伤害", 0),
				safe_get<std::string>(refNode, "str攻击音效", ""),
				safe_get_duration(refNode, "dura后摇", 0),
				safe_get<bool>(refNode, "b空中", false),
				safe_get<bool>(refNode, "b可打空中", false),
				safe_get<bool>(refNode, "b可打地面", false)
			};
			return true;
		}
	};
	template<>
	struct convert<单位::活动单位配置> {
		static Node encode(const 单位::活动单位配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::活动单位配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { 
				safe_get<std::string>(refNode, "入场语音", ""),
				safe_get<单位::动作>(refNode, "走路动作", 默认动作),
				safe_get<std::string>(refNode, "普通走语音", ""),
				safe_get<std::string>(refNode, "强行走语音", ""),
			};
			return true;
		}
	};
	template<>
	struct convert<单位::建筑单位配置> {
		static Node encode(const 单位::建筑单位配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::建筑单位配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			static const 单位::动作 默认建造动作 = {"", 1.0f, 0.0f, 0.0f};
			rhs = { 
				safe_get<float>(refNode, "f半边长", 0.0f), 
				safe_get<单位::动作>(refNode, "建造动作", 默认建造动作) 
			};
			return true;
		}
	};

	template<>
	struct convert<单位::消耗资源> {
		static Node encode(const 单位::消耗资源& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::消耗资源& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { 
				safe_get<uint16_t>(refNode, "消耗晶体矿", 0),
				safe_get<uint16_t>(refNode, "消耗燃气矿", 0) 
			};
			return true;
		}
	};

	template<>
	struct convert<单位::制造配置> {
		static Node encode(const 单位::制造配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::制造配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { 
				refNode.as<单位::消耗资源>(),
				safe_get<uint16_t>(refNode, "初始HP", 0), 
				safe_get<单位类型>(refNode, "前置单位", 单位类型(0)),
				safe_get<uint16_t>(refNode, "耗时帧", 0) 
			};
			return true;
		}
	};

	template<>
	struct convert<单位::怪配置> {
		static Node encode(const 单位::怪配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::怪配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { safe_get<uint16_t>(refNode, "u16初始Hp", 0) };
			return true;
		}
	};

	template<>
	struct convert<单位::单位属性等级配置> {
		static Node encode(const 单位::单位属性等级配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::单位属性等级配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs[safe_get<属性类型>(refNode, "属性", 属性类型(0))][safe_get<uint16_t>(refNode, "等级", 0)] = { 
				safe_get<float>(refNode, "数值", 0.0f), 
				refNode.as<单位::消耗资源>() 
			};
			return true;
		}
	};

	template<> struct convert<单位::Buff配置> {
		static Node encode(const 单位::Buff配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::Buff配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { 
				safe_get<属性类型>(refNode, "属性", 属性类型(0)), 
				safe_get<float>(refNode, "变化值", 0.0f), 
				safe_get_duration16(refNode, "间隔时长", 0) 
			};
			return true;
		}
	};

	template<>
	struct convert<单位::战局配置> {
		static Node encode(const 单位::战局配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::战局配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { 
				safe_get<战局类型>(refNode, "类型", 战局类型(0)), 
				safe_get<std::string>(refNode, "strSceneName", ""), 
				safe_get<std::string>(refNode, "寻路文件", ""), 
				safe_get<std::string>(refNode, "Https音乐", "") 
			};
			return true;
		}
	};
}

namespace 单位
{
	std::unordered_map<单位类型, 单位配置> g_map单位配置;
	std::unordered_map<单位类型, 制造配置> g_map制造配置;
	std::unordered_map<单位类型, 活动单位配置> g_map活动单位配置;
	std::unordered_map<单位类型, 建筑单位配置> g_map建筑单位配置;
	std::unordered_map<单位类型, 战斗配置> g_map战斗配置;
	std::unordered_map<单位类型, 怪配置> g_map怪配置;
	std::unordered_map<单位类型, 单位属性等级配置> g_map单位属性等级配置;
	std::unordered_map<单位类型, 消耗资源> g_map单位解锁配置;
	std::unordered_map<BuffId, Buff配置> g_mapBuff配置;
	std::unordered_map<战局类型, 战局配置> g_map副本配置;
	
	template<typename K, typename T>
	bool 读配置文件(const std::string& strPathName, std::unordered_map<K, T>& map, const std::string& strKeyName = "类型")
	{
		try {
			std::ifstream file(strPathName);
			if (!file.is_open())
				return false;
			std::ostringstream oss;
			oss << file.rdbuf();
			std::string strUtf8(oss.str());
			file.close();
			const auto strGbk = StrConv::Utf8ToGbk(strUtf8);
			YAML::Node config = YAML::Load(strGbk);
			for (auto it = config.begin(); it != config.end(); it++)
			{
				auto line = *it;
				const auto obj = line.as<T>();
				const auto 类型 = line[strKeyName].as<K>();
				if (map.end() != map.find(类型))
				{
					LOG(ERROR) << "重复的类型:" << 类型;
					_ASSERT(false);
				}
				map[类型] = obj;
				LOG(INFO) << 类型 << "," << obj;
			}
			return true;
		}
		catch (YAML::BadFile& e) {
			LOG(ERROR) << "read error!" << e.msg;
			return false;
		}
	}


	bool 读配置文件()
	{
		CHECK_RET_FALSE(读配置文件("配置/单位.yaml", g_map单位配置));
		CHECK_RET_FALSE(读配置文件("配置/制造.yaml", g_map制造配置));
		CHECK_RET_FALSE(读配置文件("配置/建筑单位.yaml", g_map建筑单位配置));
		CHECK_RET_FALSE(读配置文件("配置/活动单位.yaml", g_map活动单位配置));
		CHECK_RET_FALSE(读配置文件("配置/战斗.yaml", g_map战斗配置));
		CHECK_RET_FALSE(读配置文件("配置/怪.yaml", g_map怪配置));
		CHECK_RET_FALSE(读配置文件("配置/单位属性等级.yaml", g_map单位属性等级配置));
		CHECK_RET_FALSE(读配置文件("配置/单位解锁.yaml", g_map单位解锁配置));
		CHECK_RET_FALSE(读配置文件("配置/Buff.yaml", g_mapBuff配置, "ID"));
		CHECK_RET_FALSE(读配置文件("配置/战局.yaml", g_map副本配置));
		return true;
	}
	template<typename K, typename V>
	bool FindMap(const typename std::unordered_map<K, V>& map, const K 类型, V& refOut)
	{
		auto iterFind = map.find(类型);
		CHECK_RET_FALSE(map.end() != iterFind);
		refOut = iterFind->second;
		return true;
	}

	bool Find建筑单位配置(const 单位类型 类型, 建筑单位配置& refOut) { return FindMap(g_map建筑单位配置, 类型, refOut); }
	bool Find活动单位配置(const 单位类型 类型, 活动单位配置& refOut) { return FindMap(g_map活动单位配置, 类型, refOut); }
	bool Find单位配置(const 单位类型 类型, 单位配置& refOut) { return FindMap(g_map单位配置, 类型, refOut); }
	bool Find战斗配置(const 单位类型 类型, 战斗配置& refOut) { return FindMap(g_map战斗配置, 类型, refOut); }
	bool Find怪配置(const 单位类型 类型, 怪配置& refOut) { return FindMap(g_map怪配置, 类型, refOut); }
	bool Find制造配置(const 单位类型 类型, 制造配置& refOut) { return FindMap(g_map制造配置, 类型, refOut); }
	bool FindBuff配置(const BuffId 类型, Buff配置& refOut) { return FindMap(g_mapBuff配置, 类型, refOut); }
	bool Find单位解锁配置(const 单位类型 单位, 消耗资源& refOut) { return FindMap(g_map单位解锁配置, 单位, refOut); }
	bool Find战局配置(const 战局类型 类型, 战局配置& refOut) { return FindMap(g_map副本配置, 类型, refOut); }

	bool Find单位属性等级配置(const 单位类型 单位, const 属性类型 属性, const uint16_t u16等级, 单位属性等级配置详情& refOut)
	{
		const auto iterFind单位 = g_map单位属性等级配置.find(单位);
		if (iterFind单位 == g_map单位属性等级配置.end())
			return false;

		const auto iterFind属性 = iterFind单位->second.find(属性);
		if (iterFind属性 == iterFind单位->second.end())
			return false;

		const auto iterFind加数值 = iterFind属性->second.find(u16等级);
		if (iterFind加数值 == iterFind属性->second.end())
			return false;

		refOut = iterFind加数值->second;
		return true;
	}

	template<typename T_成员>
	T_成员 单位升级后属性(const 单位类型 单位, const 属性类型 属性, const uint16_t u16等级, T_成员 战斗配置::* p成员)
	{
		战斗配置 战斗;
		CHECK_RET_DEFAULT(Find战斗配置(单位, 战斗));

		单位::单位属性等级配置详情 等级详情 = {};
		Find单位属性等级配置(单位, 属性, u16等级, 等级详情);
		return 战斗.*p成员 + (T_成员)等级详情.f数值;
	}
	template float 单位升级后属性(const 单位类型 单位, const 属性类型 属性, const uint16_t u16等级, float 战斗配置::* p成员);
	template uint16_t 单位升级后属性(const 单位类型 单位, const 属性类型 属性, const uint16_t u16等级, uint16_t 战斗配置::* p成员);

	bool Is虫(const 单位类型 单位)
	{
		单位配置 配置;
		if (!Find单位配置(单位, 配置))
			return false;

		return 虫 == 配置.种族;
	}

	bool 战局配置::Is多人战局()const
	{
		return 战局类型::多人ID_非法_MIN < this->id副本 && this->id副本 < 战局类型::多人ID_非法_MAX;
	}
}