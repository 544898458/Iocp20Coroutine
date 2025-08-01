#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
#include "../CoRoutine/CoTask.h"
#include "枚举/MsgId.h"
#include "Position.h"
class Entity;

/// <summary>
/// 网页强制要求协议:外层是二进制WS(WebSocket)，二进制用 MsgPack 序列化
/// 微信小程序强制要求协议：外层是WSS，也就是三层，最外层TLS1.3，中间是二进制WS(WebSocket)，最里面是 MsgPack 序列化
/// </summary>
enum MsgId :uint16_t;
MSGPACK_ADD_ENUM(MsgId);

enum 单位类型;
MSGPACK_ADD_ENUM(单位类型);

enum 属性类型;
MSGPACK_ADD_ENUM(属性类型);
extern const std::initializer_list<const 属性类型> g_list所有属性;

enum 种族
{
	无,//资源
	人,
	虫,
	神,
};
MSGPACK_ADD_ENUM(种族);

enum BuffId;
MSGPACK_ADD_ENUM(BuffId);

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
	mutable uint8_t sn;//可以用这个防伪造数据，比如用伪随机序列算法生成序列
	//mutable uint32_t rpcSnId;

	MSGPACK_DEFINE(id, sn);
};

struct MsgLogin
{
	MsgHead msg;
	mutable uint16_t rpcSnId;

	std::string name;
	std::string pwd;
	uint32_t u32版本号;
	MSGPACK_DEFINE(msg, rpcSnId, name, pwd, u32版本号);
};

struct MsgLoginResponce
{
	MsgHead msg = { .id = MsgId::Login };
	mutable uint16_t rpcSnId;

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
	uint32_t uSvrId;
	MSGPACK_DEFINE(msg, rpcSnId, result, str提示, uSvrId);
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
	mutable uint16_t rpcSnId;

	bool addMoney;
	int32_t changeMoney;
	MSGPACK_DEFINE(msg, rpcSnId, addMoney, changeMoney);
};

struct MsgChangeMoneyResponce
{
	MsgHead msg{ .id = ChangeMoneyResponce };
	mutable uint16_t rpcSnId;

	/// <summary>
	/// 1上一个DB协程还没结束
	/// </summary>
	int error = 0;
	uint32_t consumeMoney;
	int32_t finalMoney;
	MSGPACK_DEFINE(msg, rpcSnId, error, consumeMoney, finalMoney);
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
	单位类型 类型;
	int16_t	最大生命;
	int16_t	最大能量;
	MSGPACK_DEFINE(msg, entityId, nickName, entityName, prefabName, 类型, 最大生命, 最大能量);
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
	MsgNotifyPos(const Entity& ref);
	MsgHead msg{ .id = NotifyPos };
	uint64_t entityId;
	Position pos;
	int16_t eulerAnglesY;
	MSGPACK_DEFINE(msg, entityId, pos, eulerAnglesY);
};
struct MsgChangeSkeleAnim
{
	MsgChangeSkeleAnim(Entity& ref, const std::string& refName, bool loop, float f播放速度, float f动作起始时刻秒, float f动作结束时刻秒) :entityId((uint64_t)&ref), loop(loop), clipName(refName), f播放速度(f播放速度), f动作起始时刻秒(f动作起始时刻秒), f动作结束时刻秒(f动作结束时刻秒) {}
	MsgHead msg{ .id = ChangeSkeleAnim };
	uint64_t entityId;
	bool loop;
	std::string clipName;
	float f播放速度;
	float f动作起始时刻秒;
	float f动作结束时刻秒;
	MSGPACK_DEFINE(msg, entityId, loop, clipName, f播放速度, f动作起始时刻秒, f动作结束时刻秒);
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
	/// <summary>
	/// 不序列化，临时存数据
	/// </summary>
	uint64_t idGateClientSession = 0;
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
	mutable uint16_t rpcSnId;
	MSGPACK_DEFINE(msg, rpcSnId);
};

struct MsgGateDeleteSessionResponce
{
	MsgHead msg{ .id = GateDeleteSessionResponce };
	mutable uint16_t rpcSnId;
	MSGPACK_DEFINE(msg, rpcSnId);
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

struct Msg进房虫
{
	MsgHead msg{ .id = 进房虫 };
	double id目标房虫;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, id目标房虫);
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
	uint32_t 建筑单位;
	uint32_t 建筑单位上限;
	MSGPACK_DEFINE(msg, 晶体矿, 燃气矿, 活动单位, 活动单位上限, 建筑单位, 建筑单位上限);
};

enum 战局类型
{
	单人ID_非法_MIN,
	新手训练_单位介绍_人,
	新手训练_单位介绍_虫,
	新手训练_反空降战_人,
	新手训练_空降战_虫,
	新手训练_战斗_人,
	新手训练_战斗_虫,
	防守战_人,
	防守战_虫,
	攻坚战_人,
	攻坚战_虫,
	中央防守_人,
	单人ID_非法_MAX,

	多人ID_非法_MIN = 100,
	四方对战,
	一打一,
	多人ID_非法_MAX,

	多人混战ID_非法_MIN = 200,
	多玩家混战,
	多玩家混战_圆坑,
	多人混战ID_非法_MAX
};
MSGPACK_ADD_ENUM(战局类型);

struct Msg进Space
{
	MsgHead msg{ .id = 进Space };
	战局类型 战局;
	std::string _;//  占位符
	bool b已看完激励视频广告;
	MSGPACK_DEFINE(msg, 战局, _, b已看完激励视频广告);
};

struct Msg进单人剧情副本
{
	MsgHead msg{ .id = 进单人剧情副本 };
	战局类型 类型;
	MSGPACK_DEFINE(msg, 类型);
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
	战局类型 _;
	std::string nickName其他玩家;
	MSGPACK_DEFINE(msg, _, nickName其他玩家);
};

struct Msg创建多人战局
{
	MsgHead msg{ .id = 创建多人战局 };
	战局类型 类型;
	MSGPACK_DEFINE(msg, 类型);
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
	战局类型 _;
	std::string nickName其他玩家;
	MSGPACK_DEFINE(msg, _, nickName其他玩家);
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
	单位类型 单位;
	MSGPACK_DEFINE(msg, pos, 单位);
};

struct Msg原地坚守
{
	MsgHead msg{ .id = 原地坚守 };
	MSGPACK_DEFINE(msg);
};

struct Msg解锁单位
{
	MsgHead msg{ .id = 解锁单位 };
	单位类型 类型;
	MSGPACK_DEFINE(msg, 类型);
};

struct Msg已解锁单位
{
	MsgHead msg{ .id = 已解锁单位 };
	std::map<单位类型, bool> map解锁状态;
	MSGPACK_DEFINE(msg, map解锁状态);
};

using MAP_单位属性等级 = std::map<单位类型, std::map<属性类型, uint16_t> >;
struct Msg单位属性等级
{
	MsgHead msg{ .id = 所有单位属性等级 };
	MAP_单位属性等级 map单位属性等级;
	MSGPACK_DEFINE(msg, map单位属性等级);
};

struct Msg升级单位属性
{
	MsgHead msg{ .id = 升级单位属性 };
	单位类型 单位;
	属性类型 属性;
	MSGPACK_DEFINE(msg, 单位, 属性);
};

struct Msg苔蔓半径
{
	Msg苔蔓半径(Entity& refEntity);
	MsgHead msg{ .id = 苔蔓半径 };
	uint64_t idEntity;
	int16_t 半径;
	MSGPACK_DEFINE(msg, idEntity, 半径);
};

struct Msg太岁分裂
{
	MsgHead msg{ .id = 太岁分裂 };
	Position pos放置;
	MSGPACK_DEFINE(msg, pos放置);
};

struct MsgNotify属性
{
	MsgNotify属性(Entity& ref, std::initializer_list<const 属性类型> list);
	void TryAdd属性(Entity& ref, const 属性类型 属性);

	MsgHead msg{ .id = Notify属性 };
	uint64_t idEntity;
	std::map<属性类型, float> map属性;
	MSGPACK_DEFINE(msg, idEntity, map属性);
};

//GameSvr=>WorldSvr
struct Msg战局结束
{
	MsgHead msg{ .id = MsgId::战局结束 };
	uint32_t uSvrId;
	std::string nickName;
	战局类型 战局类型;
	bool is赢 = false;
	MSGPACK_DEFINE(msg, uSvrId, nickName, 战局类型, is赢);
};

//GameSvr=>WorldSvr
struct Msg击杀
{
	MsgHead msg{ .id = MsgId::击杀 };
	uint32_t uSvrId;
	战局类型 战局;
	std::string nickName攻击;
	单位类型 单位类型攻击;
	std::string nickName阵亡;
	单位类型 单位类型阵亡;	
	MSGPACK_DEFINE(msg, uSvrId, 战局, nickName攻击, 单位类型攻击, nickName阵亡, 单位类型阵亡);
};

struct Msg跟随
{
	MsgHead msg{ .id = 跟随 };
	uint64_t idEntity目标;
	MSGPACK_DEFINE(msg, idEntity目标);
};

struct Msg取消跟随
{
	MsgHead msg{ .id = MsgId::取消跟随 };
	MSGPACK_DEFINE(msg);
};