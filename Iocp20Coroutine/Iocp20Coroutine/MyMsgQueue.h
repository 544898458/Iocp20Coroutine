#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
#include "../CoRoutine/CoTask.h"

class Entity;

struct Position
{
	float x;
	float z;

	bool operator==(const Position& refRight)const
	{
		return x == refRight.x && z == refRight.z;
	}
};
template <class _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const Position& _ref)
{
	return _Ostr << "(" << _ref.x << "," << _ref.z << ")";
}
enum MsgId
{
	Invalid_0,
	Login,
	Move,
	AddRoleRet,
	NotifyPos,
	ChangeSkeleAnim,
	Say,
	SelectRoles,
	AddRole,
	DelRoleRet,
	ChangeMoney,
	ChangeMoneyResponce,
	AddBuilding,
	NotifyeMoney,
	Gate转发,
	GateAddSession,
	GateAddSessionResponce,
	GateDeleteSession,
	GateDeleteSessionResponce,
	采集,
	资源,
};
MSGPACK_ADD_ENUM(MsgId);

enum 建筑单位类型
{
	基地,//指挥中心(Command Center),用来造工程车()
	兵厂,//兵营Barracks，用来造兵
	民房,//供给站(Supply Depot)
};
MSGPACK_ADD_ENUM(建筑单位类型);

enum 资源类型
{
	晶体矿,//Minerals
	燃气矿,//Vespene Gas
};
MSGPACK_ADD_ENUM(资源类型);

enum 活动单位类型
{
	工程车,//空间工程车Space Construction Vehicle。可以采矿，采气，也可以简单攻击
	兵,//陆战队员Marine。只能攻击
};
MSGPACK_ADD_ENUM(活动单位类型);

struct MsgHead
{
	//void SetSn(uint32_t snParam) const
	//{
	//	const_cast<Msg*>(this)->sn = snParam;
	//}
	static MsgHead GetMsgId(msgpack::object obj)
	{
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
	MSGPACK_DEFINE(msg, name, pwd);
};

struct MsgLoginResponce
{
	MsgHead msg = { .id = MsgId::Login };
	enum Error
	{
		OK,
		Busy,
		PwdErr,
	};
	Error  result = OK;

	MSGPACK_DEFINE(msg, result);
};
MSGPACK_ADD_ENUM(MsgLoginResponce::Error);

struct MsgAddRole
{
	MsgHead msg;
	MSGPACK_DEFINE(msg);
};

struct MsgAddBuilding
{
	MsgHead msg;
	建筑单位类型 类型;
	MSGPACK_DEFINE(msg, 类型);
};

struct MsgChangeMoney
{
	MsgHead msg{ .id = ChangeMoney };
	bool addMoney;
	uint32_t changeMoney;
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
	float x;
	float z;
	MSGPACK_DEFINE(msg, x, z);
};

struct MsgSay
{
	MsgSay(const std::string& strContent) :content(strContent) {}
	MsgSay() {}
	MsgHead msg{ .id = MsgId::Say };
	std::string content;
	MSGPACK_DEFINE(msg, content);
};

struct MsgSelectRoles
{
	MsgSelectRoles() {}
	MsgHead msg{ .id = MsgId::Say };
	std::vector<double> ids;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, ids);
};

struct MsgAddRoleRet
{
	MsgAddRoleRet(Entity&);
	MsgHead msg{ .id = AddRoleRet };
	uint64_t entityId;
	std::string nickName;
	std::string entityName;
	std::string prefabName;
	MSGPACK_DEFINE(msg, entityId, nickName, entityName, prefabName);
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
	int hp;
	MSGPACK_DEFINE(msg, entityId, x, z, eulerAnglesY, hp);
};
struct MsgChangeSkeleAnim
{
	MsgChangeSkeleAnim(Entity& ref, std::string name, bool loop = true) :entityId((uint64_t)&ref), loop(loop), clipName(name) {}
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

struct Msg资源
{
	MsgHead msg{ .id = 资源 };
	uint32_t 燃气矿;
	uint32_t 活动单位;
	uint32_t 活动单位上限;
	MSGPACK_DEFINE(msg, 燃气矿, 活动单位, 活动单位上限);
};