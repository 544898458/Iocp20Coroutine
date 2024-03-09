#pragma once
#include <msgpack.hpp>
#include <mutex>
class Entity;

struct Position
{
	float x;
	float z;
};

enum MsgId
{
	Login,
	Move,
	LoginRet,
	NotifyPos,
	ChangeSkeleAnim,
};
struct MsgLogin
{
	int id;
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(id, name, pwd);
};

struct MsgMove
{
	int id;
	float x;
	float y;
	float z;
	MSGPACK_DEFINE(id, x, y, z);
};

struct MsgLoginRet
{
	int id = (int)LoginRet;
	uint64_t entityId;
	std::string nickName;
	MSGPACK_DEFINE(id, entityId, nickName);
};

struct MsgNotifyPos
{
	MsgNotifyPos(Entity* p, float argX, float argZ):entityId((uint64_t)p),x(argX),z(argZ){}
	int msgId = (int)NotifyPos;
	uint64_t entityId;
	float x;
	float z;
	MSGPACK_DEFINE(msgId, entityId, x, z);
};
struct MsgChangeSkeleAnim 
{
	MsgChangeSkeleAnim(Entity *p,std::string name):entityId((uint64_t)p),clipName(name){}
	int msgId = (int)ChangeSkeleAnim;
	uint64_t entityId;
	std::string clipName;
	MSGPACK_DEFINE(msgId, entityId, clipName);
};

class MySession;
class MsgQueue
{
public:
	MsgQueue(MySession* p) :m_pSession(p)
	{
	}
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void Push(const MsgLogin& msg);
	void Push(const MsgMove& msg);

	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgMove& msg);
	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;
	std::deque<MsgId> m_queueMsgId;

	std::mutex m_mutex;
private:
	/// <summary>
	/// �����ã���Ҫ����
	/// </summary>
	MySession* const m_pSession;
};