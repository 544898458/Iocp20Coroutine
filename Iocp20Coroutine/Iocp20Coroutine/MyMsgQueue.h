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
	ComsumeMoney,
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

struct MsgComsumeMoney
{
	MsgId id = ComsumeMoney;
	int rpcSnId;
	uint32_t consumeMoney;
	MSGPACK_DEFINE(id, rpcSnId, consumeMoney);
};

struct MsgComsumeMoneyResponce
{
	MsgId id;
	int rpcSnId;
	int error;
	uint32_t consumeMoney;
	uint32_t finalMoney;
	MSGPACK_DEFINE(id, rpcSnId, error, consumeMoney, finalMoney);
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

class MySession;

class MyMsgQueue
{
public:
	MyMsgQueue(MySession* p) :m_pSession(p)
	{
	}

	/// <summary>
	/// 网络线程中（多线程）调用
	/// </summary>
	/// <param name="msg"></param>
	template<class T>
	void Push(const T& msg);
	//void Push(const MsgLogin& msg);
	//void Push(const MsgMove& msg);
	//void Push(const MsgSay& msg);

	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	static void OnRecv(MyMsgQueue& refThis, const MsgLogin& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgMove& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgSay& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgSelectRoles& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgAddRole& msg);
	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
	CoTask<int> CoAddRole();
private:
	template<class T>
	std::deque<T>& GetQueue();
	/// <summary>
	/// 这里保存的都是解析后的消息明文
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgSelectRoles> m_queueSelectRoles;
	std::deque<MsgAddRole> m_queueAddRole;
	/// <summary>
	/// 弱引用，不要销毁
	/// </summary>
	MySession* const m_pSession;
	MsgQueue m_MsgQueue;
};