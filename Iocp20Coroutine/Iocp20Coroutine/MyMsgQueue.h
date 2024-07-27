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
	Say,
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
	float z;
	MSGPACK_DEFINE(id, x, z);
};

struct MsgSay
{
	MsgSay(const std::string &strContent):content(strContent){}
	MsgSay() {}
	MsgId id = MsgId::Say;
	std::string content;
	MSGPACK_DEFINE(id, content);
};
struct MsgLoginRet
{
	MsgLoginRet(uint64_t entityId, std::string nickName, std::string prefabName) 
		:entityId(entityId), nickName(nickName), prefabName(prefabName){}
	MsgId id = LoginRet;
	uint64_t entityId;
	std::string nickName;
	std::string prefabName;
	MSGPACK_DEFINE(id, entityId, nickName, prefabName);
};

struct MsgNotifyPos
{
	MsgNotifyPos(Entity* p);
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
	MsgChangeSkeleAnim(Entity* p, std::string name, bool loop=true) :entityId((uint64_t)p), loop(loop), clipName(name) {}
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
	void Push(const MsgLogin& msg);
	void Push(const MsgMove& msg);
	void Push(const MsgSay& msg);

	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	static void OnRecv(MyMsgQueue &refThis, const MsgLogin& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgMove& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgSay& msg);
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
	std::deque<MsgSay> m_queueSay;

	/// <summary>
	/// 弱引用，不要销毁
	/// </summary>
	MySession* const m_pSession;
	MsgQueue m_MsgQueue;
};