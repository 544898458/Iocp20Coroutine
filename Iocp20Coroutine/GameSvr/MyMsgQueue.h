#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
#include "../CoRoutine/CoTask.h"

class Entity;
struct Position
{
	float x;
	float z;
	MSGPACK_DEFINE(x, z);
	bool operator==(const Position& refRight)const
	{
		return x == refRight.x && z == refRight.z;
	}
	void operator+=(const Position& refRight)
	{
		x += refRight.x;
		z += refRight.z;
	}
	Position operator-(const Position& refRight)const
	{
		Position pos(*this);
		pos.x -= refRight.x;
		pos.z -= refRight.z;
		return pos;
	}
	Position operator*(float f)const;
	Position operator+(const Position& refRight)const
	{
		Position pos(*this);
		pos.x += refRight.x;
		pos.z += refRight.z;
		return pos;
	}
	bool DistanceLessEqual(const Position& refPos, float fDistance)const;

	float DistancePow2(const Position& refPos)const;
	float Distance(const Position& refPos)const;

	float LengthPow2() const;

	float Length() const;
	
	/// <summary>
	/// 向量归一化(Vector Normalization)
	/// </summary>
	Position 归一化()const;
};
struct Rect
{
	Position pos左上;
	Position pos右下;
	float 宽()const
	{
		return pos右下.x - pos左上.x;
	}
	int32_t 宽Int32()const
	{
		return (int32_t)宽();
	}
	float 高()const
	{
		return pos右下.z - pos左上.z;
	}
	int32_t 高Int32()const
	{
		return (int32_t)高();
	}
	bool 包含此点(const Position& pos)const
	{
		return
			pos左上.x < pos.x && pos.x < pos右下.x &&
			pos左上.z < pos.z && pos.z < pos右下.z;
	}
};

template <class _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const Position& _ref)
{
	return _Ostr << "(" << _ref.x << "," << _ref.z << ")";
}

/// <summary>
/// 网页强制要求协议:外层是二进制WS(WebSocket)，二进制用 MsgPack 序列化
/// 微信小程序强制要求协议：外层是WSS，也就是三层，最外层TLS1.3，中间是二进制WS(WebSocket)，最里面是 MsgPack 序列化
/// </summary>
enum MsgId
{
	/// <summary>
	/// 无效
	/// </summary>
	MsgId_Invalid_0,
	/// <summary>
	/// 登录 
	/// C=>S MsgLogin
	/// S=>C 回应MsgLoginResponce
	/// </summary>
	Login,
	/// <summary>
	/// 请求把选中的单位移动到目标位置
	/// C=>S MsgMove
	/// </summary>
	Move,
	/// <summary>
	/// 场景中加单位，通知前端
	/// S=>C MsgAddRoleRet
	/// </summary>
	AddRoleRet,
	/// <summary>
	/// 通知前端，单位位置和血量变化
	/// S=>C MsgNotifyPos
	/// </summary>
	NotifyPos,
	/// <summary>
	/// 通知前端，单位播放骨骼动画（动作）
	/// S=>C MsgChangeSkeleAnim
	/// </summary>
	ChangeSkeleAnim,
	/// <summary>
	/// 系统提示和玩家聊天
	/// S=>C C=>S MsgSay 
	/// </summary>
	Say,
	/// <summary>
	/// 玩家请求选中单位
	/// C=>S	MsgSelectRoles
	/// </summary>
	SelectRoles,
	/// <summary>
	/// 玩家请求造活动单位（训练兵、近战兵，制造工程车、坦克）
	/// C=>S MsgAddRole
	/// </summary>
	AddRole,
	/// <summary>
	/// 通知前端删除一个单位
	/// S=>C MsgDelRoleRet
	/// </summary>
	DelRoleRet,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	ChangeMoney,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	ChangeMoneyResponce,
	/// <summary>
	/// 玩家请求建造建筑单位
	/// C=>S MsgAddBuilding
	/// </summary>
	AddBuilding,
	/// <summary>
	/// 通知前端钱变化（现在钱没用，前端不会显示，所以不用做。从策划上来说，钱属于局外数据）
	/// </summary>
	NotifyeMoney,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	Gate转发,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateAddSession,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateAddSessionResponce,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateDeleteSession,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateDeleteSessionResponce,
	/// <summary>
	/// 玩家请求采集
	/// </summary>
	采集,
	/// <summary>
	/// 通知前端，燃气矿、晶体矿、活动单位数
	/// Msg资源
	/// </summary>
	资源,
	/// <summary>
	/// 玩家请求选中的单位进地堡
	/// </summary>
	进地堡,
	/// <summary>
	/// 玩家请求选中的地堡中的兵全部出来
	/// </summary>
	出地堡,
	/// <summary>
	/// 玩家请求进公共地图（多人联机地图）
	/// C=>S 成功后 S=>C
	/// 注意：进单人剧情副本成功、进多人对局成功，也是发给前端 Msg进Space
	/// </summary>
	进Space,
	/// <summary>
	/// 玩家请求进单人剧情副本（训练战、防守战）
	/// </summary>
	进单人剧情副本,
	/// <summary>
	/// 没用
	/// </summary>
	显示界面_没用到,
	/// <summary>
	/// 玩家请求离开公共地图（多人联机地图）
	/// </summary>
	离开Space,
	/// <summary>
	/// 通知前端显示单位描述（建造进度、训练活动单位进度、采集进度）
	/// </summary>
	Entity描述,
	/// <summary>
	/// 通知前端播放语音或音效
	/// </summary>
	播放声音,
	/// <summary>
	/// 让玩家摄像机对准某处
	/// </summary>
	设置视口,
	/// <summary>
	/// 玩家请求框选单位
	/// C=>S
	/// </summary>
	框选,
	/// <summary>
	/// 玩家拉取全局个人战局列表
	/// </summary>
	玩家个人战局列表,
	/// <summary>
	/// 玩家请求进别人的个人战局（训练战、防守战）
	/// </summary>
	进其他玩家个人战局,
	/// <summary>
	/// 创建多人战局（四方对战）
	/// </summary>
	创建多人战局,
	/// <summary>
	/// 拉取全局多人战局列表
	/// </summary>
	玩家多人战局列表,
	/// <summary>
	/// 进别人的多人战局
	/// </summary>
	进其他玩家多人战局,
	/// <summary>
	/// 选中空闲工程车
	/// </summary>
	切换空闲工程车,
	/// <summary>
	/// 通知前端播放炮台攻击特效（从炮台飞向目标）
	/// </summary>
	弹丸特效,
	/// <summary>
	/// 通知前端显示剧情对话界面
	/// S=>C
	/// </summary>
	剧情对话,
	/// <summary>
	/// 前端告诉服务器已经看完此页剧情对话
	/// C=>S
	/// </summary>
	剧情对话已看完,
	在线人数,
	GateSvr转发GameSvr消息给游戏前端,
	GateSvr转发WorldSvr消息给游戏前端,
	建筑产出活动单位的集结点,
	播放音乐,
	原地坚守,
};
MSGPACK_ADD_ENUM(MsgId);

enum 单位类型
{
	单位类型_Invalid_0,

	特效,
	视口,

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

	活动单位Max非法,

	建筑Min非法 = 300,
	基地,//指挥中心(Command Center),可造工程车
	兵营,//兵营(Barracks)，造兵、近战兵、坦克（不需要重工厂）
	民房,//供给站(Supply Depot)
	地堡,//掩体; 地堡(Bunker),可以进兵
	炮台,//Photon Cannon
	虫巢,//hatchery
	机场,//Spaceport
	重工厂,//Factory 
	虫营,//对应兵营
	飞塔,//Spore Conlony

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
MSGPACK_ADD_ENUM(单位类型);


struct MsgHead
{
	//void SetSn(uint32_t snParam) const
	//{
	//	const_cast<Msg*>(this)->sn = snParam;
	//}
	static MsgHead GetMsgId(msgpack::object obj)
	{
		if (obj.type != MSGPACK_OBJECT_ARRAY)
		{
			LOG(WARNING) << "数据包头类型错:" << obj.type;
			return {};
		}
		if (obj.via.array.size < 1)
		{
			LOG(WARNING) << "数据包头数组太小:" << obj.via.array.size;
			return {};
		}
		return obj.via.array.ptr[0].as<MsgHead>();
		//return obj.as<Msg>();
		//return (MsgId)obj.via.array.ptr[0].via.array.ptr[0].via.i64;//没判断越界，要加try
	}
	MsgId id;
	mutable uint32_t sn;//可以用这个防伪造数据，比如用伪随机序列算法生成序列
	mutable uint32_t rpcSnId;

	MSGPACK_DEFINE(id, sn, rpcSnId);
};

struct MsgLogin
{
	MsgHead msg;
	std::string name;
	std::string pwd;
	uint32_t u32版本号;
	MSGPACK_DEFINE(msg, name, pwd, u32版本号);
};

struct MsgLoginResponce
{
	MsgHead msg = { .id = MsgId::Login };
	enum Error
	{
		OK,
		Busy,
		PwdErr,
		NameErr,
		客户端版本不匹配,
	};
	Error  result = OK;
	std::string str提示;
	MSGPACK_DEFINE(msg, result, str提示);
};
MSGPACK_ADD_ENUM(MsgLoginResponce::Error);

struct MsgAddRole
{
	MsgHead msg;
	单位类型 类型;
	MSGPACK_DEFINE(msg, 类型);
};

struct MsgAddBuilding
{
	MsgHead msg;
	单位类型 类型;
	Position pos;
	MSGPACK_DEFINE(msg, 类型, pos);
};

struct MsgChangeMoney
{
	MsgHead msg{ .id = ChangeMoney };
	bool addMoney;
	int32_t changeMoney;
	MSGPACK_DEFINE(msg, addMoney, changeMoney);
};

struct MsgChangeMoneyResponce
{
	MsgHead msg{ .id = ChangeMoneyResponce };
	/// <summary>
	/// 1上一个DB协程还没结束
	/// </summary>
	int error = 0;
	uint32_t consumeMoney;
	int32_t finalMoney;
	MSGPACK_DEFINE(msg, error, consumeMoney, finalMoney);
};

struct MsgNotifyMoney
{
	MsgHead msg{ .id = NotifyeMoney };
	int32_t finalMoney;
	MSGPACK_DEFINE(msg, finalMoney);
};

struct MsgMove
{
	MsgHead msg;
	Position pos;
	bool b遇到敌人自动攻击;
	MSGPACK_DEFINE(msg, pos, b遇到敌人自动攻击);
};

enum SayChannel
{
	系统,
	聊天,
	任务提示,
};
MSGPACK_ADD_ENUM(SayChannel);

struct MsgSay
{
	MsgHead msg{ .id = MsgId::Say };
	std::string content;

	SayChannel channel = 系统;
	MSGPACK_DEFINE(msg, content, channel);
};


struct MsgSelectRoles
{
	MsgSelectRoles() {}
	MsgHead msg{ .id = MsgId::SelectRoles };
	std::vector<double> ids;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	bool b追加 = false;
	MSGPACK_DEFINE(msg, ids, b追加);
};

struct MsgAddRoleRet
{
	MsgAddRoleRet(Entity&);
	MsgHead msg{ .id = AddRoleRet };
	uint64_t entityId;
	std::string nickName;
	std::string entityName;
	std::string prefabName;
	int32_t	i32HpMax;
	单位类型 类型;
	MSGPACK_DEFINE(msg, entityId, nickName, entityName, prefabName, i32HpMax, 类型);
};

struct MsgDelRoleRet
{
	MsgDelRoleRet(uint64_t entityId) :entityId(entityId) {}
	MsgHead msg{ .id = DelRoleRet };
	uint64_t entityId;
	MSGPACK_DEFINE(msg, entityId);
};

struct MsgNotifyPos
{
	MsgNotifyPos(Entity& ref);
	MsgHead msg{ .id = NotifyPos };
	uint64_t entityId;
	float x;
	float z;
	int eulerAnglesY;
	int hp = 0;
	MSGPACK_DEFINE(msg, entityId, x, z, eulerAnglesY, hp);
};
struct MsgChangeSkeleAnim
{
	MsgChangeSkeleAnim(Entity& ref, const std::string& refName, bool loop) :entityId((uint64_t)&ref), loop(loop), clipName(refName) {}
	MsgHead msg{ .id = ChangeSkeleAnim };
	uint64_t entityId;
	bool loop;
	std::string clipName;
	MSGPACK_DEFINE(msg, entityId, loop, clipName);
};

struct MsgGate转发
{
	MsgGate转发()
	{

	}
	MsgGate转发(const void* buf, int len, uint64_t uGateClientSessionId, uint32_t snParam) :vecByte(len), gateClientSessionId(uGateClientSessionId)
	{
		msg.sn = snParam;
		uint8_t* pBegin = (uint8_t*)buf;
		std::copy(pBegin, pBegin + len, vecByte.begin());
	}
	MsgHead msg{ .id = Gate转发 };
	uint64_t gateClientSessionId;
	std::vector<uint8_t> vecByte;
	MSGPACK_DEFINE(msg, gateClientSessionId, vecByte);
};

struct MsgGateSvr转发GameSvr消息给游戏前端
{
	MsgGateSvr转发GameSvr消息给游戏前端()
	{

	}
	MsgGateSvr转发GameSvr消息给游戏前端(const void* buf, int len) :vecByte(len)
	{
		uint8_t* pBegin = (uint8_t*)buf;
		std::copy(pBegin, pBegin + len, vecByte.begin());
	}
	MsgHead msg{ .id = GateSvr转发GameSvr消息给游戏前端 };
	std::vector<uint8_t> vecByte;
	MSGPACK_DEFINE(msg, vecByte);
};
struct MsgGateSvr转发WorldSvr消息给游戏前端
{
	MsgGateSvr转发WorldSvr消息给游戏前端()
	{

	}
	MsgGateSvr转发WorldSvr消息给游戏前端(const void* buf, int len) :vecByte(len)
	{
		uint8_t* pBegin = (uint8_t*)buf;
		std::copy(pBegin, pBegin + len, vecByte.begin());
	}
	MsgHead msg{ .id = GateSvr转发WorldSvr消息给游戏前端 };
	std::vector<uint8_t> vecByte;
	MSGPACK_DEFINE(msg, vecByte);
};


struct MsgGateAddSession
{
	MsgHead msg{ .id = GateAddSession };
	std::string nickName;

	MSGPACK_DEFINE(msg, nickName);
};

struct MsgGateDeleteSession
{
	MsgHead msg{ .id = GateDeleteSession };
	MSGPACK_DEFINE(msg);
};

struct MsgGateDeleteSessionResponce
{
	MsgHead msg{ .id = GateDeleteSessionResponce };
	MSGPACK_DEFINE(msg);
};

struct Msg采集
{
	MsgHead msg{ .id = 采集 };
	double id目标资源;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, id目标资源);
};

struct Msg进地堡
{
	MsgHead msg{ .id = 进地堡 };
	double id目标地堡;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, id目标地堡);
};

struct Msg出地堡
{
	MsgHead msg{ .id = 出地堡 };
	double id地堡;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	std::vector<uint64_t> vecId地堡内单位;
	MSGPACK_DEFINE(msg, id地堡, vecId地堡内单位);
};

struct Msg资源
{
	MsgHead msg{ .id = 资源 };
	uint32_t 晶体矿;
	uint32_t 燃气矿;
	uint32_t 活动单位;
	uint32_t 活动单位上限;
	MSGPACK_DEFINE(msg, 晶体矿, 燃气矿, 活动单位, 活动单位上限);
};

struct Msg进Space
{
	MsgHead msg{ .id = 进Space };
	uint32_t idSapce;
	MSGPACK_DEFINE(msg, idSapce);
};

enum 副本ID
{
	单人ID_非法_MIN,
	训练战,
	防守战,
	攻坚战,
	训练战_虫,
	防守战_虫,
	单人ID_非法_MAX,

	多人ID_非法_MIN = 100,
	四方对战,
	多人ID_非法_MAX,

	多人联机地图,
};
MSGPACK_ADD_ENUM(副本ID);

struct Msg进单人剧情副本
{
	MsgHead msg{ .id = 进单人剧情副本 };
	副本ID id;
	MSGPACK_DEFINE(msg, id);
};

struct Msg显示界面
{
	MsgHead msg{ .id = 显示界面_没用到 };
	enum 界面类型
	{
		选择地图,
	};
	界面类型 ui;
	MSGPACK_DEFINE(msg, ui);
};
MSGPACK_ADD_ENUM(Msg显示界面::界面类型);


struct Msg离开Space
{
	MsgHead msg{ .id = 离开Space };
	MSGPACK_DEFINE(msg);
};

struct MsgEntity描述
{
	MsgHead msg{ .id = Entity描述 };
	uint64_t idEntity;
	std::string str描述;
	MSGPACK_DEFINE(msg, idEntity, str描述);
};

struct Msg播放声音
{
	MsgHead msg{ .id = 播放声音 };
	std::string str声音;
	std::string str文本;
	MSGPACK_DEFINE(msg, str声音, str文本);
};
struct Msg播放网络音乐
{
	MsgHead msg{ .id = 播放音乐 };
	std::string strHttpsMp3;
	MSGPACK_DEFINE(msg, strHttpsMp3);
};

struct Msg设置视口
{
	MsgHead msg{ .id = 设置视口 };
	Position pos视口;
	MSGPACK_DEFINE(msg, pos视口);
};

struct Msg框选
{
	MsgHead msg{ .id = 框选 };
	Position pos起始;//都是世界坐标
	Position pos结束;
	MSGPACK_DEFINE(msg, pos起始, pos结束);
};

struct Msg玩家个人战局列表
{
	MsgHead msg{ .id = 玩家个人战局列表 };
	MSGPACK_DEFINE(msg);
};


struct 玩家场景
{
	std::string nickName;
	std::string sceneName;
	MSGPACK_DEFINE(nickName, sceneName);
};

struct Msg玩家个人战局列表Responce
{
	MsgHead msg{ .id = 玩家个人战局列表 };
	std::vector<玩家场景> vec个人战局中的玩家;
	MSGPACK_DEFINE(msg, vec个人战局中的玩家);
};

struct Msg进其他玩家个人战局
{
	MsgHead msg{ .id = 进其他玩家个人战局 };
	std::string nickName其他玩家;
	MSGPACK_DEFINE(msg, nickName其他玩家);
};

struct Msg创建多人战局
{
	MsgHead msg{ .id = 创建多人战局 };
	副本ID id;
	MSGPACK_DEFINE(msg, id);
};

struct Msg玩家多人战局列表
{
	MsgHead msg{ .id = 玩家多人战局列表 };
	MSGPACK_DEFINE(msg);
};

struct Msg玩家多人战局列表Responce
{
	MsgHead msg{ .id = 玩家多人战局列表 };
	std::vector<玩家场景> vec多人战局中的Host玩家;
	MSGPACK_DEFINE(msg, vec多人战局中的Host玩家);
};

struct Msg进其他玩家多人战局
{
	MsgHead msg{ .id = 进其他玩家多人战局 };
	std::string nickName其他玩家;
	MSGPACK_DEFINE(msg, nickName其他玩家);
};

struct Msg切换空闲工程车
{
	MsgHead msg{ .id = 切换空闲工程车 };
	MSGPACK_DEFINE(msg);
};

struct Msg弹丸特效
{
	//Msg弹丸特效(Entity& ref, const Position& refTarget) :entityId(ref.Id), pos起始(ref.Pos()) {}
	MsgHead msg{ .id = 弹丸特效 };
	uint64_t idEntity;
	uint64_t idEntityTarget;
	std::string 特效name;
	MSGPACK_DEFINE(msg, idEntity, idEntityTarget, 特效name);
};

struct Msg剧情对话
{
	MsgHead msg{ .id = 剧情对话 };
	std::string str头像左;
	std::string str名字左;
	std::string str头像右;
	std::string str名字右;
	std::string str对话内容;
	bool b显示退出场景按钮;
	MSGPACK_DEFINE(msg, str头像左, str名字左, str头像右, str名字右, str对话内容, b显示退出场景按钮);
};
struct Msg剧情对话已看完
{
	MsgHead msg{ .id = 剧情对话已看完 };
	MSGPACK_DEFINE(msg);
};

struct Msg在线人数
{
	MsgHead msg{ .id = 在线人数 };
	uint16_t u16人数;
	std::vector<std::string> vec玩家NickName;
	MSGPACK_DEFINE(msg, u16人数, vec玩家NickName);
};

struct Msg建筑产出活动单位的集结点
{
	MsgHead msg{ .id = 建筑产出活动单位的集结点 };
	Position pos;
	MSGPACK_DEFINE(msg, pos);
};

struct Msg原地坚守
{
	MsgHead msg{ .id = 原地坚守 };
	MSGPACK_DEFINE(msg);
};
