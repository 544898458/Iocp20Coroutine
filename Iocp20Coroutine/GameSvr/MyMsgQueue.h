#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
#include "../CoRoutine/CoTask.h"
#include "枚举/MsgId.h"
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
enum MsgId;
MSGPACK_ADD_ENUM(MsgId);

enum 单位类型;
MSGPACK_ADD_ENUM(单位类型);

enum 属性类型;
MSGPACK_ADD_ENUM(属性类型);

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
	攻坚战_虫,
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
	MsgHead msg{ .id = 太岁分裂};
	Position pos放置;
	MSGPACK_DEFINE(msg, pos放置);
};