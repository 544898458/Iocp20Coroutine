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
	MsgId id = MsgId::Say;
	std::string content;
	MSGPACK_DEFINE(id, content);
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
	MsgNotifyPos(Entity* p, float argX, float argZ, int argEulerAnglesY, int argHp) :
		entityId((uint64_t)p), x(argX), z(argZ), eulerAnglesY(argEulerAnglesY),hp(argHp) {}
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
	/// �����߳��У����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void Push(const MsgLogin& msg);
	void Push(const MsgMove& msg);
	void Push(const MsgSay& msg);

	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	static void OnRecv(MyMsgQueue &refThis, const MsgLogin& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgMove& msg);
	static void OnRecv(MyMsgQueue& refThis, const MsgSay& msg);
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
private:
	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;
	std::deque<MsgSay> m_queueSay;

	/// <summary>
	/// �����ã���Ҫ����
	/// </summary>
	MySession* const m_pSession;
	MsgQueue m_MsgQueue;
};