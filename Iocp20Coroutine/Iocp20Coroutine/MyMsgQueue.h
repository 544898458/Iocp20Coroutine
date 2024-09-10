#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
#include "../CoRoutine/CoTask.h"

class Entity;

struct Position
{
	float x;
	float z;
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
	ConsumeMoney,
	ChangeMoneyResponce,
	AddBuilding,
	NotifyeMoney,
	Gate转发,
	GateAddSession,
	GateDeleteSession,
};
MSGPACK_ADD_ENUM(MsgId);

struct Msg
{
	void SetSn(uint32_t snParam) const
	{
		const_cast<Msg*>(this)->sn = snParam;
	}
	static Msg GetMsgId(msgpack::object obj)
	{
		return obj.via.array.ptr[0].as<Msg>();
		//return obj.as<Msg>();
		//return (MsgId)obj.via.array.ptr[0].via.array.ptr[0].via.i64;//没判断越界，要加try
	}
	MsgId id;
	uint32_t sn;//可以用这个防伪造数据，比如用伪随机序列算法生成序列
	MSGPACK_DEFINE(id, sn);
};

struct MsgLogin
{
	Msg msg;
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(msg, name, pwd);
};

struct MsgAddRole
{
	Msg msg;
	MSGPACK_DEFINE(msg);
};

struct MsgAddBuilding
{
	Msg msg;
	MSGPACK_DEFINE(msg);
};

struct MsgChangeMoney
{
	Msg msg{ .id = ConsumeMoney };
	int rpcSnId;
	bool addMoney;
	int32_t changeMoney;
	std::string nickName;
	MSGPACK_DEFINE(msg, rpcSnId, addMoney, changeMoney, nickName);
};

struct MsgChangeMoneyResponce
{
	Msg msg{ .id = ChangeMoneyResponce };
	int rpcSnId;
	int error = 0;
	uint32_t consumeMoney;
	int32_t finalMoney;
	MSGPACK_DEFINE(msg, rpcSnId, error, consumeMoney, finalMoney);
};

struct MsgNotifyMoney
{
	Msg msg{ .id = NotifyeMoney };
	int32_t finalMoney;
	MSGPACK_DEFINE(msg, finalMoney);
};

struct MsgMove
{
	Msg msg;
	float x;
	float z;
	MSGPACK_DEFINE(msg, x, z);
};

struct MsgSay
{
	MsgSay(const std::string& strContent) :content(strContent) {}
	MsgSay() {}
	Msg msg{ .id = MsgId::Say };
	std::string content;
	MSGPACK_DEFINE(msg, content);
};

struct MsgSelectRoles
{
	MsgSelectRoles() {}
	Msg msg{ .id = MsgId::Say };
	std::vector<double> ids;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, ids);
};

struct MsgAddRoleRet
{
	MsgAddRoleRet(uint64_t entityId, std::string nickName, std::string prefabName)
		:entityId(entityId), nickName(nickName), prefabName(prefabName) {}
	Msg msg{ .id = AddRoleRet };
	uint64_t entityId;
	std::string nickName;
	std::string prefabName;
	MSGPACK_DEFINE(msg, entityId, nickName, prefabName);
};

struct MsgDelRoleRet
{
	MsgDelRoleRet(uint64_t entityId) :entityId(entityId) {}
	Msg msg{ .id = DelRoleRet };
	uint64_t entityId;
	MSGPACK_DEFINE(msg, entityId);
};

struct MsgNotifyPos
{
	MsgNotifyPos(Entity& ref);
	Msg msg{ .id = NotifyPos };
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
	Msg msg{ .id = ChangeSkeleAnim };
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
	Msg msg{ .id = Gate转发 };
	uint64_t gateClientSessionId;
	std::vector<uint8_t> vecByte;
	MSGPACK_DEFINE(msg, gateClientSessionId, vecByte);
};

struct MsgGateAddSession
{
	Msg msg{ .id = GateAddSession };
	uint64_t gateClientSessionId;
	//IP地址
	MSGPACK_DEFINE(msg, gateClientSessionId);
};

struct MsgGateDeleteSession
{
	Msg msg{ .id = GateDeleteSession };
	uint64_t gateClientSessionId;
	MSGPACK_DEFINE(msg, gateClientSessionId);
};

struct DbTest
{
	uint32_t id = 0;
	int a;
};