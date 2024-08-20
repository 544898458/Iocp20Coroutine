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
};
MSGPACK_ADD_ENUM(MsgId);

struct MsgLogin
{
	MsgId id;
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(id, name, pwd);
};

struct MsgAddRole
{
	MsgId id;
	MSGPACK_DEFINE(id);
};

struct MsgAddBuilding
{
	MsgId id;
	MSGPACK_DEFINE(id);
};

struct MsgChangeMoney
{
	MsgId id = ConsumeMoney;
	int rpcSnId;
	bool addMoney;
	int32_t changeMoney;
	std::string nickName;
	MSGPACK_DEFINE(id, rpcSnId, addMoney, changeMoney, nickName);
};

struct MsgChangeMoneyResponce
{
	MsgId id = ChangeMoneyResponce;
	int rpcSnId;
	int error = 0;
	uint32_t consumeMoney;
	int32_t finalMoney;
	MSGPACK_DEFINE(id, rpcSnId, error, consumeMoney, finalMoney);
};

struct MsgNotifyMoney
{
	MsgId id = NotifyeMoney;
	int32_t finalMoney;
	MSGPACK_DEFINE(id, finalMoney);
};

struct MsgMove
{
	MsgId id;
	float x;
	float z;
	MSGPACK_DEFINE(id, x, z);
};

struct MsgSay
{
	MsgSay(const std::string& strContent) :content(strContent) {}
	MsgSay() {}
	MsgId id = MsgId::Say;
	std::string content;
	MSGPACK_DEFINE(id, content);
};

struct MsgSelectRoles
{
	MsgSelectRoles() {}
	MsgId id = MsgId::Say;
	std::vector<double> ids;//TypeScript只有FLOAT64,没有POSITIVE_INTEGER和NEGATIVE_INTEGER
	MSGPACK_DEFINE(id, ids);
};

struct MsgAddRoleRet
{
	MsgAddRoleRet(uint64_t entityId, std::string nickName, std::string prefabName)
		:entityId(entityId), nickName(nickName), prefabName(prefabName) {}
	MsgId id = AddRoleRet;
	uint64_t entityId;
	std::string nickName;
	std::string prefabName;
	MSGPACK_DEFINE(id, entityId, nickName, prefabName);
};

struct MsgDelRoleRet
{
	MsgDelRoleRet(uint64_t entityId) :entityId(entityId) {}
	MsgId id = DelRoleRet;
	uint64_t entityId;
	MSGPACK_DEFINE(id, entityId);
};

struct MsgNotifyPos
{
	MsgNotifyPos(Entity& ref);
	MsgId msgId = NotifyPos;
	uint64_t entityId;
	float x;
	float z;
	int eulerAnglesY;
	int hp;
	MSGPACK_DEFINE(msgId, entityId, x, z, eulerAnglesY, hp);
};
struct MsgChangeSkeleAnim
{
	MsgChangeSkeleAnim(Entity& ref, std::string name, bool loop = true) :entityId((uint64_t)&ref), loop(loop), clipName(name) {}
	MsgId msgId = ChangeSkeleAnim;
	uint64_t entityId;
	bool loop;
	std::string clipName;
	MSGPACK_DEFINE(msgId, entityId, loop, clipName);
};