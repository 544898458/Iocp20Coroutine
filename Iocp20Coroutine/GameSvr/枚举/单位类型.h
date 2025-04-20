#pragma once
enum 单位类型
{
	单位类型_Invalid_0,

	特效,
	视口,
	苔蔓,	//Creep

	资源Min非法 = 100,
	晶体矿,//Minerals
	燃气矿,//Vespene Gas
	资源Max非法,

	活动单位Min非法 = 200,
	工程车,//空间工程车Space Construction Vehicle。可以采矿，采气，也可以简单攻击
	枪兵,//陆战队员Marine。只能攻击，不能采矿
	近战兵,//火蝠，喷火兵Firebat
	三色坦克,//！不是！攻城坦克（Siege Tank）
	工虫,//Drone
	飞机,
	枪虫,//Hydralisk
	近战虫,//Zergling
	幼虫,//Larva
	绿色坦克,//虫群单位，实际上是生物体
	光刺,//由绿色坦克发射，直线前进，遇敌爆炸
	房虫,//overload
	飞虫,//Mutalisk
	医疗兵,//Medic,

	活动单位Max非法,

	建筑Min非法 = 300,
	基地,//指挥中心(Command Center),可造工程车
	兵营,//兵营(Barracks)，造兵、近战兵、坦克（不需要重工厂）
	民房,//供给站(Supply Depot)
	地堡,//掩体; 地堡(Bunker),可以进兵
	炮台,//Photon Cannon
	虫巢,//hatchery
	机场,//Spaceport
	重车厂,//Factory 
	虫营,//对应兵营
	飞塔,//Spore Conlony
	拟态源,//拟态源，原创，绿色坦克前置建筑
	太岁,	//Creep Colony

	建筑Max非法,

	怪Min非法 = 400,
	枪虫怪,
	近战虫怪,
	工虫怪,
	枪兵怪,
	近战兵怪,
	工程车怪,
	怪Max非法,
};