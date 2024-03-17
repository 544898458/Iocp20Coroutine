#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
class Entity;

struct Position
{
	float x;
	float z;
};

enum MsgId
{
	Invalid_0,
	Login,
	Move,
	LoginRet,
	NotifyPos,
	ChangeSkeleAnim,
};
MSGPACK_ADD_ENUM(MsgId);

struct MsgLogin
{
	MsgId id;
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(id, name, pwd);
};

struct MsgMove
{
	MsgId id;
	float x;
	float y;
	float z;
	MSGPACK_DEFINE(id, x, y, z);
};

struct MsgLoginRet
{
	MsgId id = LoginRet;
	uint64_t entityId;
	std::string nickName;
	MSGPACK_DEFINE(id, entityId, nickName);
};

struct MsgNotifyPos
{
	MsgNotifyPos(Entity* p, float argX, float argZ) :entityId((uint64_t)p), x(argX), z(argZ) {}
	MsgId msgId = NotifyPos;
	uint64_t entityId;
	float x;
	float z;
	MSGPACK_DEFINE(msgId, entityId, x, z);
};
struct MsgChangeSkeleAnim
{
	MsgChangeSkeleAnim(Entity* p, std::string name) :entityId((uint64_t)p), clipName(name) {}
	MsgId msgId = ChangeSkeleAnim;
	uint64_t entityId;
	std::string clipName;
	MSGPACK_DEFINE(msgId, entityId, clipName);
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
	void Push(const MsgLogin& msg);
	void Push(const MsgMove& msg);

	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	static void OnRecv(MyMsgQueue* pThis, const MsgLogin& msg);
	static void OnRecv(MyMsgQueue* pThis, const MsgMove& msg);
	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
private:
	/// <summary>
	/// 这里保存的都是解析后的消息明文
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;

	/// <summary>
	/// 弱引用，不要销毁
	/// </summary>
	MySession* const m_pSession;
	MsgQueue m_MsgQueue;
};