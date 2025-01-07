#include "pch.h"
#include "单位.h"
#include <unordered_map>
namespace 单位
{

	std::unordered_map<建筑单位类型, 建筑单位配置> g_map建筑单位配置 =
	{
		//								选中音效		晶  燃 HP	半边长
		{基地, {{"基地","建筑/基地",	"tcsWht00"},	{50, 0,200}, 5.0f}},
		{兵厂, {{"兵厂","建筑/兵厂",	"tacWht00"},	{25, 9,100}, 4.0f} },
		{民房, {{"民房","建筑/民房",	"tclWht00"},	{20, 3,50},  3.f} },
		{地堡, {{"地堡","建筑/地堡",	"音效/BUTTON"},	{30,11,500}, 3.0f} },
	};
	using namespace std;
	std::unordered_map<活动单位类型, 活动单位配置> g_map活动单位配置 =
	{
		//											选中音效			  晶 燃 HP 警距 攻距 伤害 移速 前摇动作 dura开始播放攻击动作 攻击动作 dura开始伤害 攻击音效		  阵亡音效	阵亡动作
		{工程车,{{"工程车"	, "altman-yellow",		"语音/是女声可爱版"},{ 3, 0,20},{8 , 4,  1,	0.4f,		"",		0s,				  "attack",		500ms,	 "TSCMin00",	 "TSCDth00","died"}}},
		{兵,	{{"兵"		, "活动单位/步兵",		"语音/是男声正经版"},{ 5, 2,15},{20, 9,  3,	0.5f,		"",		0s,				  "attack",		900ms,	 "音效/TTaFir00","TMaDth00","die01"}}},
		{近战兵,{{"近战兵"	, "axe-yellow",			"tfbPss00"},		 { 8, 3,18},{15, 4,  5,	0.5f,		"",		0s,				  "attack",		600ms,	 "Tfrshoot",	 "TMaDth00","died"}}},
		{三色坦克,{{"三色坦克","活动单位/三色坦克",	"语音/ttapss00"},	 {20,10,60},{35,25,	30,	0.1f,	"idle",		5s,			 "attack_loop",		300ms,	 "音效/TTaFi200","音效/TTaDth00","die02"}}},
	};
	std::unordered_map<活动单位类型, 怪配置> g_map怪配置 = {
		//								 警距 攻距 伤  移速 前摇动作 dura开始播放攻击动作 攻击动作 dura开始伤害 攻击音效	阵亡音效 阵亡动作 血量
		{兵,	{{"怪"	, "altman-red"	}, {20,8,	5, 0.2f,	"",			0s,			  "attack",	300ms,		"TMaFir00","TMaDth00","died"}, 50} },
		{近战兵,{{"跳虫", "活动单位/跳虫"},{15,3,	8, 0.4f,	"",			0s,			  "attack",	400ms, "音效/利爪攻击","TMaDth00","died"}, 65} },
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