#include "pch.h"
#include "单位.h"
#include <unordered_map>
namespace 单位
{

	std::unordered_map<单位类型, 建筑单位配置> g_map建筑单位配置 =
	{
		//									选中音效		晶  燃 HP	半边长
		{基地,	{{"基地",	"建筑/基地",	"tcsWht00"},	{50, 0,300}, 5.0f}},
		{兵厂,	{{"兵厂",	"建筑/兵厂",	"tacWht00"},	{25, 9,100}, 4.0f}},
		{民房,	{{"民房",	"建筑/民房",	"tclWht00"},	{20, 3,50},  3.f}},
		{地堡,	{{"地堡",	"建筑/地堡",	"音效/BUTTON"},	{30,11,500}, 3.0f}},
		{光子炮,{{"光子炮",	"建筑/光子炮",	"音效/BUTTON"},	{35,12,100}, 2.5f}},
		{孵化场,{{"孵化场",	"建筑/孵化场",	"音效/BUTTON"},	{35,12,200}, 5.0f}},
	};
	using namespace std;
	std::unordered_map<单位类型, 活动单位配置> g_map活动单位配置 =
	{
		//											选中音效			  晶 燃 HP 警距 攻距 伤害 移速 前摇动作 dura开始播放攻击动作 攻击动作 dura开始伤害	攻击音效		阵亡音效	阵亡动作	后摇	入场语音
		{工程车,{{"工程车"	, "活动单位/工程车",	"语音/是女声可爱版"},{ 3, 0,25},{10, 5,  1,	0.4f,		"",		0s,				"attack",		500ms,	"音效/TScFir00","TSCDth00",		"died", 100ms},	"语音/工程车准备就绪女声可爱版"}},
		{工蜂,	{{"工蜂"	, "活动单位/工蜂",		"语音/zdrYes00"},	 { 2, 0,15},{10, 5,	 2,	0.4f,		"",		0s,				"attack",		600ms,	"音效/ZDrHit00","语音/zdrDth00","died", 600ms},	"语音/zdrRdy00"}},
		{兵,	{{"兵"		, "活动单位/步兵",		"语音/是男声正经版"},{ 5, 2,15},{20,10,  3,	0.5f,		"",		0s,				"attack",		900ms,	"音效/TTaFir00","TMaDth00",		"die01",100ms},	"语音/准备战斗男声正经版"}},
		{近战兵,{{"近战兵"	, "axe-yellow",			"tfbPss00"},		 { 8, 3,18},{15, 5,  5,	0.5f,		"",		0s,				"attack",		600ms,	"Tfrshoot",		"TMaDth00",		"died",	100ms},	"tfbRdy00"}},
		{三色坦克,{{"三色坦克","活动单位/三色坦克",	"语音/ttapss00"},	 {20,10,60},{35,25,	30,	0.1f,	"idle",		5s,				"attack_loop",	300ms,	"音效/TTaFi200","音效/TTaDth00","die02",100ms},	"语音/ttardy00"}},
	};
	std::unordered_map<单位类型, 怪配置> g_map怪配置 = {
		//									警距 攻距 伤 移速 前摇动作 dura开始播放攻击动作 攻击动作	dura开始伤害 攻击音效			阵亡音效		阵亡动作 后摇 血量
		{兵,	{{"刺蛇", "活动单位/刺蛇"}, {20, 9,	  3, 0.2f,	"",			0s,				"attack",		300ms,	"音效/发射骨针","音效/zhyDth00",	"died"		}, 50} },
		{近战兵,{{"跳虫", "活动单位/跳虫"},	{15, 5,	  5, 0.35f,	"",			0s,				"attack",		400ms,	"音效/利爪攻击","音效/ZZeDth00",	"died"		}, 60} },
		{工蜂,	{{"工蜂", "活动单位/工蜂"},	{10, 5,	  2, 0.3f,	"",			0s,				"attack",		400ms,	"音效/ZDrHit00","语音/zdrDth00",	"died"		}, 25} },
		{三色坦克,{{"三色坦克", "活动单位/三色坦克"},{35,25,30,0.1f,"idle",	5s,				"attack_loop",	300ms,	"音效/TTaFi200","音效/TTaDth00",	"die02",100ms},60} },
	};

	std::unordered_map<单位类型, 资源单位配置> g_map资源单位配置 =
	{
		{晶体矿,{{"晶体矿"	, "资源/晶体矿"	}}},
		{燃气矿,{{"燃气矿"	, "资源/燃气矿"}}},
	};

	template<class K, class V>
	bool Find配置(std::unordered_map<K, V>& refMap, const K 类型, V& refOut)
	{
		const auto& iterFind = refMap.find(类型);
		if (iterFind == refMap.end())
		{
			_ASSERT(false);
			return false;
		}

		refOut = iterFind->second;
		return true;
	}
	bool Find建筑单位配置(const 单位类型 类型, 建筑单位配置& refOut)
	{
		return Find配置(g_map建筑单位配置, 类型, refOut);
	}

	bool Find活动单位配置(const 单位类型 类型, 活动单位配置& refOut)
	{
		return Find配置(g_map活动单位配置, 类型, refOut);
	}
	bool Find资源单位配置(const 单位类型 类型, 资源单位配置& refOut)
	{
		return Find配置(g_map资源单位配置, 类型, refOut);
	}
	bool Find怪配置(const 单位类型 类型, 怪配置& refOut)
	{
		return Find配置(g_map怪配置, 类型, refOut);
	}
}