#include "pch.h"
#include "单位.h"
#include <unordered_map>

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#ifdef _DEBUG
#pragma comment(lib, "../yaml-cpp-0.8.0/lib/yaml-cppd.lib")
#else
#pragma comment(lib, "../yaml-cpp-0.8.0/lib/yaml-cpp.lib")
#endif

namespace std
{
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::单位配置& _ref)
	{
		return _Ostr << "单位配置:" << _ref.strName << "\t" << _ref.strPrefabName << "\t" << _ref.str选中音效;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::战斗配置& _ref)
	{
		return _Ostr << "战斗配置," << _ref.f警戒距离 << "\t" << _ref.f攻击距离 << "\t" << _ref.i32伤害 << "\t" << _ref.f每帧移动距离
			<< _ref.str前摇动作 << "\t" << _ref.dura开始播放攻击动作 << "\t" << _ref.str攻击动作 << "\t" << _ref.dura开始伤害
			<< _ref.str攻击音效 << "\t" << _ref.str阵亡音效 << "\t" << _ref.str阵亡动作 << "\t" << _ref.dura后摇;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::活动单位配置& _ref)
	{
		return _Ostr << "活动单位配置," << _ref.str入场语音;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::建筑单位配置& _ref)
	{
		return _Ostr << "建筑单位配置," << _ref.f半边长;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::制造配置& _ref)
	{
		return _Ostr << "制造配置," << _ref.u16消耗晶体矿 << "\t" << _ref.u16消耗燃气矿 << "\t" << _ref.u16初始Hp;
	}
	template <class _Traits>
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const 单位::怪配置& _ref)
	{
		return _Ostr << "怪配置," << _ref.u16初始Hp;
	}
}

namespace YAML {
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
	struct convert<单位::单位配置> {
		static Node encode(const 单位::单位配置& rhs) {

			LOG(ERROR) << "";
			_ASSERT(false);
			return Node();
		}
		static bool decode(const Node& refNode, 单位::单位配置& rhs) {
			CHECK_RET_FALSE(refNode.IsMap());
			rhs = { refNode["名字"].as<std::string>(), refNode["PrefabPathName"].as<std::string>(), refNode["选中音效"].as<std::string>() };
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
			rhs = { refNode["f警戒距离"].as<float>(),
					refNode["f攻击距离"].as<float>(),
					refNode["i32伤害"].as<int32_t>(),
					refNode["f每帧移动距离"].as<float>(),
					refNode["str前摇动作"].as<std::string>(),
					std::chrono::milliseconds(refNode["dura开始播放攻击动作"].as<int32_t>()),
					refNode["str攻击动作"].as<std::string>(),
					std::chrono::milliseconds(refNode["dura开始伤害"].as<int32_t>()),
					refNode["str攻击音效"].as<std::string>(),
					refNode["str阵亡音效"].as<std::string>(),
					refNode["str阵亡动作"].as<std::string>(),
					std::chrono::milliseconds(refNode["dura后摇"].as<int32_t>())
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
			rhs = { refNode["入场语音"].as<std::string>() };
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
			rhs = { refNode["f半边长"].as<float>() };
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
			rhs = { refNode["消耗晶体矿"].as<uint16_t>(), refNode["消耗燃气矿"].as<uint16_t>(), refNode["初始HP"].as<uint16_t>() };
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
			rhs = { refNode["u16初始Hp"].as<uint16_t>() };
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
	//std::unordered_map<单位类型, 建筑单位配置> g_map建筑单位配置;// =
	//{
	//	//									选中音效		晶  燃 HP	半边长
	//	{基地,	{{"基地",	"建筑/基地",	"tcsWht00"},	{50, 0,300}, 5.0f}},
	//	{兵营,	{{"兵营",	"建筑/兵厂",	"tacWht00"},	{25, 9,100}, 4.0f}},
	//	{民房,	{{"民房",	"建筑/民房",	"tclWht00"},	{20, 3,50},  3.f}},
	//	{地堡,	{{"地堡",	"建筑/地堡",	"音效/BUTTON"},	{30,11,400}, 3.0f}},
	//	{炮台,	{{"炮台",	"建筑/光子炮",	"音效/BUTTON"},	{35,12,100}, 2.5f}},
	//	{虫巢,	{{"虫巢",	"建筑/孵化场",	"音效/BUTTON"},	{35,12,200}, 5.0f}},
	//	{机场,	{{"机场",	"建筑/机场",	"音效/BUTTON"},	{35,12,100}, 4.0f}},
	//	{重工厂,{{"重车厂",	"建筑/重工厂",	"音效/BUTTON"},	{35,12,100}, 4.0f}},
	//};
	//using namespace std;
	//std::unordered_map<单位类型, 活动单位配置> g_map活动单位配置 =
	//{
	//	//											选中音效			  晶 燃 HP 警距 攻距 伤害 移速 前摇动作 dura开始播放攻击动作 攻击动作 dura开始伤害	攻击音效		阵亡音效	阵亡动作	后摇	入场语音
	//	{工程车,{{"工程车"	, "活动单位/工程车",	"语音/是女声可爱版"},{ 3, 0,25},{10, 5,  1,	0.4f,		"",		0s,				"attack",		500ms,	"音效/TScFir00","TSCDth00",		"died", 100ms},	"语音/工程车准备就绪女声可爱版"}},
	//	{兵,	{{"枪兵"	, "活动单位/步兵",		"语音/是男声正经版"},{ 5, 2,15},{20,10,  3,	0.5f,		"",		0s,				"attack",		900ms,	"音效/TTaFir00","TMaDth00",		"die01",100ms},	"语音/准备战斗男声正经版"}},
	//	{近战兵,{{"近战兵"	, "活动单位/近战兵",	"tfbPss00"},		 { 8, 3,18},{15, 5,  5,	0.6f,		"",		0s,				"attack",		600ms,	"Tfrshoot",		"TMaDth00",		"died",	100ms},	"tfbRdy00"}},
	//	{三色坦克,{{"三色坦克","活动单位/三色坦克",	"音效/坦克行进声"},	 {20,10,60},{35,25,	30,	0.1f,	"idle",		5s,				"attack_loop",	300ms,	"音效/TTaFi200","音效/TTaDth00","die02",100ms},	"语音/ttardy00"}},
	//	{工蜂,	{{"工虫"	, "活动单位/工蜂",		"语音/zdrYes00"},	 { 2, 0,15},{10, 5,	 2,	0.5f,		"",		0s,				"attack",		600ms,	"音效/ZDrHit00","语音/zdrDth00","died", 300ms},	"语音/zdrRdy00"}},
	//	{飞机,	{{"飞机"	, "活动单位/飞机",		"音效/飞机声"},		 {20,10,30},{35,15,	 3,	0.7f,		"",		0s,				"attack",		500ms,	"TMaFir00",		"音效/TTaDth00","died",	200ms},	""}},
	//	{幼虫,	{{"幼虫"	, "活动单位/幼虫",		"语音/zdrYes00"},	 {99,99,10},{ 0, 0,	 0,	0.1f,		"",		0s,				"attack",		600ms,	"音效/ZDrHit00","语音/zdrDth00","died", 300ms},	"语音/zdrRdy00"}},
	//};
	//std::unordered_map<单位类型, 怪配置> g_map怪配置 = {
	//	//									警距 攻距 伤 移速 前摇动作 dura开始播放攻击动作 攻击动作	dura开始伤害 攻击音效			阵亡音效		阵亡动作 后摇 血量
	//	{兵,	{{"远程怪", "活动单位/刺蛇"}, {20, 9,	  3, 0.2f,	"",			0s,				"attack",		300ms,	"音效/发射骨针","音效/zhyDth00",	"died"		}, 50} },
	//	{近战兵,{{"近战怪", "活动单位/跳虫"},	{15, 5,	  5, 0.35f,	"",			0s,				"attack",		400ms,	"音效/利爪攻击","音效/ZZeDth00",	"died"		}, 60} },
	//	{工蜂,	{{"工蜂", "活动单位/工蜂"},	{10, 5,	  2, 0.3f,	"",			0s,				"attack",		400ms,	"音效/ZDrHit00","语音/zdrDth00",	"died"		}, 25} },
	//	{三色坦克,{{"三色坦克", "活动单位/三色坦克"},{35,25,30,0.1f,"idle",	5s,				"attack_loop",	300ms,	"音效/TTaFi200","音效/TTaDth00",	"die02",100ms},60} },
	//};

	//std::unordered_map<单位类型, 资源单位配置> g_map资源单位配置 =
	//{
	//	{晶体矿,{{"晶体矿"	, "资源/晶体矿"	}}},
	//	{燃气矿,{{"燃气矿"	, "资源/燃气矿"}}},
	//};

	template<class T>
	bool 读配置文件(const std::string& strPathName, std::unordered_map<单位类型, T>& map)
	{
		try {
			YAML::Node config = YAML::LoadFile(strPathName);
			for (auto it = config.begin(); it != config.end(); it++)
			{
				auto line = *it;
				const auto obj = line.as<T>();
				const auto 类型 = line["类型"].as<单位类型>();
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
		return true;
	}
	template<typename V>
	bool FindMap(const typename std::unordered_map<单位类型, V>& map, const 单位类型 类型, V& refOut)
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
}